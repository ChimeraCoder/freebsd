
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

#ifndef OPENSSL_NO_SHA

#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/sha.h>
#ifndef OPENSSL_NO_DSA
#include <openssl/dsa.h>
#endif

#ifndef OPENSSL_FIPS 

static int init(EVP_MD_CTX *ctx)
	{ return SHA1_Init(ctx->md_data); }

static int update(EVP_MD_CTX *ctx,const void *data,size_t count)
	{ return SHA1_Update(ctx->md_data,data,count); }

static int final(EVP_MD_CTX *ctx,unsigned char *md)
	{ return SHA1_Final(md,ctx->md_data); }

static const EVP_MD dss1_md=
	{
	NID_dsa,
	NID_dsaWithSHA1,
	SHA_DIGEST_LENGTH,
	EVP_MD_FLAG_PKEY_DIGEST,
	init,
	update,
	final,
	NULL,
	NULL,
	EVP_PKEY_DSA_method,
	SHA_CBLOCK,
	sizeof(EVP_MD *)+sizeof(SHA_CTX),
	};

const EVP_MD *EVP_dss1(void)
	{
	return(&dss1_md);
	}
#endif
#endif