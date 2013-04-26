
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

#ifndef OPENSSL_NO_RIPEMD

#include <openssl/ripemd.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif
#include "evp_locl.h"

static int init(EVP_MD_CTX *ctx)
	{ return RIPEMD160_Init(ctx->md_data); }

static int update(EVP_MD_CTX *ctx,const void *data,size_t count)
	{ return RIPEMD160_Update(ctx->md_data,data,count); }

static int final(EVP_MD_CTX *ctx,unsigned char *md)
	{ return RIPEMD160_Final(md,ctx->md_data); }

static const EVP_MD ripemd160_md=
	{
	NID_ripemd160,
	NID_ripemd160WithRSA,
	RIPEMD160_DIGEST_LENGTH,
	0,
	init,
	update,
	final,
	NULL,
	NULL,
	EVP_PKEY_RSA_method,
	RIPEMD160_CBLOCK,
	sizeof(EVP_MD *)+sizeof(RIPEMD160_CTX),
	};

const EVP_MD *EVP_ripemd160(void)
	{
	return(&ripemd160_md);
	}
#endif