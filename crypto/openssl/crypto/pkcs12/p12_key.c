
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/pkcs12.h>
#include <openssl/bn.h>

/* Uncomment out this line to get debugging info about key generation */
/*#define DEBUG_KEYGEN*/
#ifdef DEBUG_KEYGEN
#include <openssl/bio.h>
extern BIO *bio_err;
void h__dump (unsigned char *p, int len);
#endif

/* PKCS12 compatible key/IV generation */
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

int PKCS12_key_gen_asc(const char *pass, int passlen, unsigned char *salt,
	     int saltlen, int id, int iter, int n, unsigned char *out,
	     const EVP_MD *md_type)
{
	int ret;
	unsigned char *unipass;
	int uniplen;

	if(!pass) {
		unipass = NULL;
		uniplen = 0;
	} else if (!OPENSSL_asc2uni(pass, passlen, &unipass, &uniplen)) {
		PKCS12err(PKCS12_F_PKCS12_KEY_GEN_ASC,ERR_R_MALLOC_FAILURE);
		return 0;
	}
	ret = PKCS12_key_gen_uni(unipass, uniplen, salt, saltlen,
						 id, iter, n, out, md_type);
	if (ret <= 0)
	    return 0;
	if(unipass) {
		OPENSSL_cleanse(unipass, uniplen);	/* Clear password from memory */
		OPENSSL_free(unipass);
	}
	return ret;
}

int PKCS12_key_gen_uni(unsigned char *pass, int passlen, unsigned char *salt,
	     int saltlen, int id, int iter, int n, unsigned char *out,
	     const EVP_MD *md_type)
{
	unsigned char *B, *D, *I, *p, *Ai;
	int Slen, Plen, Ilen, Ijlen;
	int i, j, u, v;
	int ret = 0;
	BIGNUM *Ij, *Bpl1;	/* These hold Ij and B + 1 */
	EVP_MD_CTX ctx;
#ifdef  DEBUG_KEYGEN
	unsigned char *tmpout = out;
	int tmpn = n;
#endif

#if 0
	if (!pass) {
		PKCS12err(PKCS12_F_PKCS12_KEY_GEN_UNI,ERR_R_PASSED_NULL_PARAMETER);
		return 0;
	}
#endif

	EVP_MD_CTX_init(&ctx);
#ifdef  DEBUG_KEYGEN
	fprintf(stderr, "KEYGEN DEBUG\n");
	fprintf(stderr, "ID %d, ITER %d\n", id, iter);
	fprintf(stderr, "Password (length %d):\n", passlen);
	h__dump(pass, passlen);
	fprintf(stderr, "Salt (length %d):\n", saltlen);
	h__dump(salt, saltlen);
#endif
	v = EVP_MD_block_size (md_type);
	u = EVP_MD_size (md_type);
	if (u < 0)
	    return 0;
	D = OPENSSL_malloc (v);
	Ai = OPENSSL_malloc (u);
	B = OPENSSL_malloc (v + 1);
	Slen = v * ((saltlen+v-1)/v);
	if(passlen) Plen = v * ((passlen+v-1)/v);
	else Plen = 0;
	Ilen = Slen + Plen;
	I = OPENSSL_malloc (Ilen);
	Ij = BN_new();
	Bpl1 = BN_new();
	if (!D || !Ai || !B || !I || !Ij || !Bpl1)
		goto err;
	for (i = 0; i < v; i++) D[i] = id;
	p = I;
	for (i = 0; i < Slen; i++) *p++ = salt[i % saltlen];
	for (i = 0; i < Plen; i++) *p++ = pass[i % passlen];
	for (;;) {
		if (!EVP_DigestInit_ex(&ctx, md_type, NULL)
			|| !EVP_DigestUpdate(&ctx, D, v)
			|| !EVP_DigestUpdate(&ctx, I, Ilen)
			|| !EVP_DigestFinal_ex(&ctx, Ai, NULL))
			goto err;
		for (j = 1; j < iter; j++) {
			if (!EVP_DigestInit_ex(&ctx, md_type, NULL)
				|| !EVP_DigestUpdate(&ctx, Ai, u)
				|| !EVP_DigestFinal_ex(&ctx, Ai, NULL))
			goto err;
		}
		memcpy (out, Ai, min (n, u));
		if (u >= n) {
#ifdef DEBUG_KEYGEN
			fprintf(stderr, "Output KEY (length %d)\n", tmpn);
			h__dump(tmpout, tmpn);
#endif
			ret = 1;
			goto end;
		}
		n -= u;
		out += u;
		for (j = 0; j < v; j++) B[j] = Ai[j % u];
		/* Work out B + 1 first then can use B as tmp space */
		if (!BN_bin2bn (B, v, Bpl1))
			goto err;
		if (!BN_add_word (Bpl1, 1))
			goto err;
		for (j = 0; j < Ilen ; j+=v) {
			if (!BN_bin2bn(I + j, v, Ij))
				goto err;
			if (!BN_add(Ij, Ij, Bpl1))
				goto err;
			if (!BN_bn2bin(Ij, B))
				goto err;
			Ijlen = BN_num_bytes (Ij);
			/* If more than 2^(v*8) - 1 cut off MSB */
			if (Ijlen > v) {
				if (!BN_bn2bin (Ij, B))
					goto err;
				memcpy (I + j, B + 1, v);
#ifndef PKCS12_BROKEN_KEYGEN
			/* If less than v bytes pad with zeroes */
			} else if (Ijlen < v) {
				memset(I + j, 0, v - Ijlen);
				if (!BN_bn2bin(Ij, I + j + v - Ijlen))
					goto err;
#endif
			} else if (!BN_bn2bin (Ij, I + j))
				goto err;
		}
	}

err:
	PKCS12err(PKCS12_F_PKCS12_KEY_GEN_UNI,ERR_R_MALLOC_FAILURE);

end:
	OPENSSL_free (Ai);
	OPENSSL_free (B);
	OPENSSL_free (D);
	OPENSSL_free (I);
	BN_free (Ij);
	BN_free (Bpl1);
	EVP_MD_CTX_cleanup(&ctx);
	return ret;
}
#ifdef DEBUG_KEYGEN
void h__dump (unsigned char *p, int len)
{
	for (; len --; p++) fprintf(stderr, "%02X", *p);
	fprintf(stderr, "\n");	
}
#endif