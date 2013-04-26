
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


#include <string.h>
#include <limits.h>

#include "cryptlib.h"

#include "ech_locl.h"
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>

static int ecdh_compute_key(void *out, size_t len, const EC_POINT *pub_key,
	EC_KEY *ecdh, 
	void *(*KDF)(const void *in, size_t inlen, void *out, size_t *outlen));

static ECDH_METHOD openssl_ecdh_meth = {
	"OpenSSL ECDH method",
	ecdh_compute_key,
#if 0
	NULL, /* init     */
	NULL, /* finish   */
#endif
	0,    /* flags    */
	NULL  /* app_data */
};

const ECDH_METHOD *ECDH_OpenSSL(void)
	{
	return &openssl_ecdh_meth;
	}


/* This implementation is based on the following primitives in the IEEE 1363 standard:
 *  - ECKAS-DH1
 *  - ECSVDP-DH
 * Finally an optional KDF is applied.
 */
static int ecdh_compute_key(void *out, size_t outlen, const EC_POINT *pub_key,
	EC_KEY *ecdh,
	void *(*KDF)(const void *in, size_t inlen, void *out, size_t *outlen))
	{
	BN_CTX *ctx;
	EC_POINT *tmp=NULL;
	BIGNUM *x=NULL, *y=NULL;
	const BIGNUM *priv_key;
	const EC_GROUP* group;
	int ret= -1;
	size_t buflen, len;
	unsigned char *buf=NULL;

	if (outlen > INT_MAX)
		{
		ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ERR_R_MALLOC_FAILURE); /* sort of, anyway */
		return -1;
		}

	if ((ctx = BN_CTX_new()) == NULL) goto err;
	BN_CTX_start(ctx);
	x = BN_CTX_get(ctx);
	y = BN_CTX_get(ctx);
	
	priv_key = EC_KEY_get0_private_key(ecdh);
	if (priv_key == NULL)
		{
		ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ECDH_R_NO_PRIVATE_VALUE);
		goto err;
		}

	group = EC_KEY_get0_group(ecdh);
	if ((tmp=EC_POINT_new(group)) == NULL)
		{
		ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	if (!EC_POINT_mul(group, tmp, NULL, pub_key, priv_key, ctx)) 
		{
		ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ECDH_R_POINT_ARITHMETIC_FAILURE);
		goto err;
		}
		
	if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field) 
		{
		if (!EC_POINT_get_affine_coordinates_GFp(group, tmp, x, y, ctx)) 
			{
			ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ECDH_R_POINT_ARITHMETIC_FAILURE);
			goto err;
			}
		}
#ifndef OPENSSL_NO_EC2M
	else
		{
		if (!EC_POINT_get_affine_coordinates_GF2m(group, tmp, x, y, ctx)) 
			{
			ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ECDH_R_POINT_ARITHMETIC_FAILURE);
			goto err;
			}
		}
#endif

	buflen = (EC_GROUP_get_degree(group) + 7)/8;
	len = BN_num_bytes(x);
	if (len > buflen)
		{
		ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ERR_R_INTERNAL_ERROR);
		goto err;
		}
	if ((buf = OPENSSL_malloc(buflen)) == NULL)
		{
		ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	
	memset(buf, 0, buflen - len);
	if (len != (size_t)BN_bn2bin(x, buf + buflen - len))
		{
		ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ERR_R_BN_LIB);
		goto err;
		}

	if (KDF != 0)
		{
		if (KDF(buf, buflen, out, &outlen) == NULL)
			{
			ECDHerr(ECDH_F_ECDH_COMPUTE_KEY,ECDH_R_KDF_FAILED);
			goto err;
			}
		ret = outlen;
		}
	else
		{
		/* no KDF, just copy as much as we can */
		if (outlen > buflen)
			outlen = buflen;
		memcpy(out, buf, outlen);
		ret = outlen;
		}
	
err:
	if (tmp) EC_POINT_free(tmp);
	if (ctx) BN_CTX_end(ctx);
	if (ctx) BN_CTX_free(ctx);
	if (buf) OPENSSL_free(buf);
	return(ret);
	}