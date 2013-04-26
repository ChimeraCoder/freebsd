
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

#ifndef OPENSSL_FIPS

#ifndef OPENSSL_NO_SHA

#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/sha.h>
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif


static int init(EVP_MD_CTX *ctx)
	{ return SHA1_Init(ctx->md_data); }

static int update(EVP_MD_CTX *ctx,const void *data,size_t count)
	{ return SHA1_Update(ctx->md_data,data,count); }

static int final(EVP_MD_CTX *ctx,unsigned char *md)
	{ return SHA1_Final(md,ctx->md_data); }

static const EVP_MD sha1_md=
	{
	NID_sha1,
	NID_sha1WithRSAEncryption,
	SHA_DIGEST_LENGTH,
	EVP_MD_FLAG_PKEY_METHOD_SIGNATURE|EVP_MD_FLAG_DIGALGID_ABSENT,
	init,
	update,
	final,
	NULL,
	NULL,
	EVP_PKEY_RSA_method,
	SHA_CBLOCK,
	sizeof(EVP_MD *)+sizeof(SHA_CTX),
	};

const EVP_MD *EVP_sha1(void)
	{
	return(&sha1_md);
	}
#endif

#ifndef OPENSSL_NO_SHA256
static int init224(EVP_MD_CTX *ctx)
	{ return SHA224_Init(ctx->md_data); }
static int init256(EVP_MD_CTX *ctx)
	{ return SHA256_Init(ctx->md_data); }
/*
 * Even though there're separate SHA224_[Update|Final], we call
 * SHA256 functions even in SHA224 context. This is what happens
 * there anyway, so we can spare few CPU cycles:-)
 */
static int update256(EVP_MD_CTX *ctx,const void *data,size_t count)
	{ return SHA256_Update(ctx->md_data,data,count); }
static int final256(EVP_MD_CTX *ctx,unsigned char *md)
	{ return SHA256_Final(md,ctx->md_data); }

static const EVP_MD sha224_md=
	{
	NID_sha224,
	NID_sha224WithRSAEncryption,
	SHA224_DIGEST_LENGTH,
	EVP_MD_FLAG_PKEY_METHOD_SIGNATURE|EVP_MD_FLAG_DIGALGID_ABSENT,
	init224,
	update256,
	final256,
	NULL,
	NULL,
	EVP_PKEY_RSA_method,
	SHA256_CBLOCK,
	sizeof(EVP_MD *)+sizeof(SHA256_CTX),
	};

const EVP_MD *EVP_sha224(void)
	{ return(&sha224_md); }

static const EVP_MD sha256_md=
	{
	NID_sha256,
	NID_sha256WithRSAEncryption,
	SHA256_DIGEST_LENGTH,
	EVP_MD_FLAG_PKEY_METHOD_SIGNATURE|EVP_MD_FLAG_DIGALGID_ABSENT,
	init256,
	update256,
	final256,
	NULL,
	NULL,
	EVP_PKEY_RSA_method,
	SHA256_CBLOCK,
	sizeof(EVP_MD *)+sizeof(SHA256_CTX),
	};

const EVP_MD *EVP_sha256(void)
	{ return(&sha256_md); }
#endif	/* ifndef OPENSSL_NO_SHA256 */

#ifndef OPENSSL_NO_SHA512
static int init384(EVP_MD_CTX *ctx)
	{ return SHA384_Init(ctx->md_data); }
static int init512(EVP_MD_CTX *ctx)
	{ return SHA512_Init(ctx->md_data); }
/* See comment in SHA224/256 section */
static int update512(EVP_MD_CTX *ctx,const void *data,size_t count)
	{ return SHA512_Update(ctx->md_data,data,count); }
static int final512(EVP_MD_CTX *ctx,unsigned char *md)
	{ return SHA512_Final(md,ctx->md_data); }

static const EVP_MD sha384_md=
	{
	NID_sha384,
	NID_sha384WithRSAEncryption,
	SHA384_DIGEST_LENGTH,
	EVP_MD_FLAG_PKEY_METHOD_SIGNATURE|EVP_MD_FLAG_DIGALGID_ABSENT,
	init384,
	update512,
	final512,
	NULL,
	NULL,
	EVP_PKEY_RSA_method,
	SHA512_CBLOCK,
	sizeof(EVP_MD *)+sizeof(SHA512_CTX),
	};

const EVP_MD *EVP_sha384(void)
	{ return(&sha384_md); }

static const EVP_MD sha512_md=
	{
	NID_sha512,
	NID_sha512WithRSAEncryption,
	SHA512_DIGEST_LENGTH,
	EVP_MD_FLAG_PKEY_METHOD_SIGNATURE|EVP_MD_FLAG_DIGALGID_ABSENT,
	init512,
	update512,
	final512,
	NULL,
	NULL,
	EVP_PKEY_RSA_method,
	SHA512_CBLOCK,
	sizeof(EVP_MD *)+sizeof(SHA512_CTX),
	};

const EVP_MD *EVP_sha512(void)
	{ return(&sha512_md); }
#endif	/* ifndef OPENSSL_NO_SHA512 */

#endif