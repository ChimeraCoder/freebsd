
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
#include <stdlib.h>
#include <string.h>
#include "cryptlib.h"
#include <openssl/cmac.h>

#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#endif

struct CMAC_CTX_st
	{
	/* Cipher context to use */
	EVP_CIPHER_CTX cctx;
	/* Keys k1 and k2 */
	unsigned char k1[EVP_MAX_BLOCK_LENGTH];
	unsigned char k2[EVP_MAX_BLOCK_LENGTH];
	/* Temporary block */
	unsigned char tbl[EVP_MAX_BLOCK_LENGTH];
	/* Last (possibly partial) block */
	unsigned char last_block[EVP_MAX_BLOCK_LENGTH];
	/* Number of bytes in last block: -1 means context not initialised */
	int nlast_block;
	};


/* Make temporary keys K1 and K2 */

static void make_kn(unsigned char *k1, unsigned char *l, int bl)
	{
	int i;
	/* Shift block to left, including carry */
	for (i = 0; i < bl; i++)
		{
		k1[i] = l[i] << 1;
		if (i < bl - 1 && l[i + 1] & 0x80)
			k1[i] |= 1;
		}
	/* If MSB set fixup with R */
	if (l[0] & 0x80)
		k1[bl - 1] ^= bl == 16 ? 0x87 : 0x1b;
	}

CMAC_CTX *CMAC_CTX_new(void)
	{
	CMAC_CTX *ctx;
	ctx = OPENSSL_malloc(sizeof(CMAC_CTX));
	if (!ctx)
		return NULL;
	EVP_CIPHER_CTX_init(&ctx->cctx);
	ctx->nlast_block = -1;
	return ctx;
	}

void CMAC_CTX_cleanup(CMAC_CTX *ctx)
	{
#ifdef OPENSSL_FIPS
	if (FIPS_mode() && !ctx->cctx.engine)
		{
		FIPS_cmac_ctx_cleanup(ctx);
		return;
		}
#endif
	EVP_CIPHER_CTX_cleanup(&ctx->cctx);
	OPENSSL_cleanse(ctx->tbl, EVP_MAX_BLOCK_LENGTH);
	OPENSSL_cleanse(ctx->k1, EVP_MAX_BLOCK_LENGTH);
	OPENSSL_cleanse(ctx->k2, EVP_MAX_BLOCK_LENGTH);
	OPENSSL_cleanse(ctx->last_block, EVP_MAX_BLOCK_LENGTH);
	ctx->nlast_block = -1;
	}

EVP_CIPHER_CTX *CMAC_CTX_get0_cipher_ctx(CMAC_CTX *ctx)
	{
	return &ctx->cctx;
	}

void CMAC_CTX_free(CMAC_CTX *ctx)
	{
	CMAC_CTX_cleanup(ctx);
	OPENSSL_free(ctx);
	}

int CMAC_CTX_copy(CMAC_CTX *out, const CMAC_CTX *in)
	{
	int bl;
	if (in->nlast_block == -1)
		return 0;
	if (!EVP_CIPHER_CTX_copy(&out->cctx, &in->cctx))
		return 0;
	bl = EVP_CIPHER_CTX_block_size(&in->cctx);
	memcpy(out->k1, in->k1, bl);
	memcpy(out->k2, in->k2, bl);
	memcpy(out->tbl, in->tbl, bl);
	memcpy(out->last_block, in->last_block, bl);
	out->nlast_block = in->nlast_block;
	return 1;
	}

int CMAC_Init(CMAC_CTX *ctx, const void *key, size_t keylen, 
			const EVP_CIPHER *cipher, ENGINE *impl)
	{
	static unsigned char zero_iv[EVP_MAX_BLOCK_LENGTH];
#ifdef OPENSSL_FIPS
	if (FIPS_mode())
		{
		/* If we have an ENGINE need to allow non FIPS */
		if ((impl || ctx->cctx.engine)
			&& !(ctx->cctx.flags & EVP_CIPH_FLAG_NON_FIPS_ALLOW))

			{
			EVPerr(EVP_F_CMAC_INIT, EVP_R_DISABLED_FOR_FIPS);
			return 0;
			}
		/* Other algorithm blocking will be done in FIPS_cmac_init,
		 * via FIPS_cipherinit().
		 */
		if (!impl && !ctx->cctx.engine)
			return FIPS_cmac_init(ctx, key, keylen, cipher, NULL);
		}
#endif
	/* All zeros means restart */
	if (!key && !cipher && !impl && keylen == 0)
		{
		/* Not initialised */
		if (ctx->nlast_block == -1)
			return 0;
		if (!EVP_EncryptInit_ex(&ctx->cctx, NULL, NULL, NULL, zero_iv))
			return 0;
		memset(ctx->tbl, 0, EVP_CIPHER_CTX_block_size(&ctx->cctx));
		ctx->nlast_block = 0;
		return 1;
		}
	/* Initialiase context */
	if (cipher && !EVP_EncryptInit_ex(&ctx->cctx, cipher, impl, NULL, NULL))
		return 0;
	/* Non-NULL key means initialisation complete */
	if (key)
		{
		int bl;
		if (!EVP_CIPHER_CTX_cipher(&ctx->cctx))
			return 0;
		if (!EVP_CIPHER_CTX_set_key_length(&ctx->cctx, keylen))
			return 0;
		if (!EVP_EncryptInit_ex(&ctx->cctx, NULL, NULL, key, zero_iv))
			return 0;
		bl = EVP_CIPHER_CTX_block_size(&ctx->cctx);
		if (!EVP_Cipher(&ctx->cctx, ctx->tbl, zero_iv, bl))
			return 0;
		make_kn(ctx->k1, ctx->tbl, bl);
		make_kn(ctx->k2, ctx->k1, bl);
		OPENSSL_cleanse(ctx->tbl, bl);
		/* Reset context again ready for first data block */
		if (!EVP_EncryptInit_ex(&ctx->cctx, NULL, NULL, NULL, zero_iv))
			return 0;
		/* Zero tbl so resume works */
		memset(ctx->tbl, 0, bl);
		ctx->nlast_block = 0;
		}
	return 1;
	}

int CMAC_Update(CMAC_CTX *ctx, const void *in, size_t dlen)
	{
	const unsigned char *data = in;
	size_t bl;
#ifdef OPENSSL_FIPS
	if (FIPS_mode() && !ctx->cctx.engine)
		return FIPS_cmac_update(ctx, in, dlen);
#endif
	if (ctx->nlast_block == -1)
		return 0;
	if (dlen == 0)
		return 1;
	bl = EVP_CIPHER_CTX_block_size(&ctx->cctx);
	/* Copy into partial block if we need to */
	if (ctx->nlast_block > 0)
		{
		size_t nleft;
		nleft = bl - ctx->nlast_block;
		if (dlen < nleft)
			nleft = dlen;
		memcpy(ctx->last_block + ctx->nlast_block, data, nleft);
		dlen -= nleft;
		ctx->nlast_block += nleft;
		/* If no more to process return */
		if (dlen == 0)
			return 1;
		data += nleft;
		/* Else not final block so encrypt it */
		if (!EVP_Cipher(&ctx->cctx, ctx->tbl, ctx->last_block,bl))
			return 0;
		}
	/* Encrypt all but one of the complete blocks left */
	while(dlen > bl)
		{
		if (!EVP_Cipher(&ctx->cctx, ctx->tbl, data, bl))
			return 0;
		dlen -= bl;
		data += bl;
		}
	/* Copy any data left to last block buffer */
	memcpy(ctx->last_block, data, dlen);
	ctx->nlast_block = dlen;
	return 1;

	}

int CMAC_Final(CMAC_CTX *ctx, unsigned char *out, size_t *poutlen)
	{
	int i, bl, lb;
#ifdef OPENSSL_FIPS
	if (FIPS_mode() && !ctx->cctx.engine)
		return FIPS_cmac_final(ctx, out, poutlen);
#endif
	if (ctx->nlast_block == -1)
		return 0;
	bl = EVP_CIPHER_CTX_block_size(&ctx->cctx);
	*poutlen = (size_t)bl;
	if (!out)
		return 1;
	lb = ctx->nlast_block;
	/* Is last block complete? */
	if (lb == bl)
		{
		for (i = 0; i < bl; i++)
			out[i] = ctx->last_block[i] ^ ctx->k1[i];
		}
	else
		{
		ctx->last_block[lb] = 0x80;
		if (bl - lb > 1)
			memset(ctx->last_block + lb + 1, 0, bl - lb - 1);
		for (i = 0; i < bl; i++)
			out[i] = ctx->last_block[i] ^ ctx->k2[i];
		}
	if (!EVP_Cipher(&ctx->cctx, out, out, bl))
		{
		OPENSSL_cleanse(out, bl);	
		return 0;
		}
	return 1;
	}

int CMAC_resume(CMAC_CTX *ctx)
	{
	if (ctx->nlast_block == -1)
		return 0;
	/* The buffer "tbl" containes the last fully encrypted block
	 * which is the last IV (or all zeroes if no last encrypted block).
	 * The last block has not been modified since CMAC_final().
	 * So reinitliasing using the last decrypted block will allow
	 * CMAC to continue after calling CMAC_Final(). 
	 */
	return EVP_EncryptInit_ex(&ctx->cctx, NULL, NULL, NULL, ctx->tbl);
	}