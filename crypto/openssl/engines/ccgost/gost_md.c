
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
#include "gost_lcl.h"
#include "gosthash.h"
#include "e_gost_err.h"

/* implementation of GOST 34.11 hash function See gost_md.c*/
static int gost_digest_init(EVP_MD_CTX *ctx);
static int gost_digest_update(EVP_MD_CTX *ctx, const void *data, size_t count);
static int gost_digest_final(EVP_MD_CTX *ctx,unsigned char *md);
static int gost_digest_copy(EVP_MD_CTX *to,const EVP_MD_CTX *from);
static int gost_digest_cleanup(EVP_MD_CTX *ctx);

EVP_MD digest_gost=  
	{
	NID_id_GostR3411_94,
	NID_undef,
	32,
	EVP_MD_FLAG_PKEY_METHOD_SIGNATURE,
	gost_digest_init,
	gost_digest_update,
	gost_digest_final,
	gost_digest_copy,
	gost_digest_cleanup,
	NULL,
	NULL,
	{NID_undef,NID_undef,0,0,0},
	32,
	sizeof(struct ossl_gost_digest_ctx ),
	NULL
	};

int gost_digest_init(EVP_MD_CTX *ctx) 
	{
	struct ossl_gost_digest_ctx *c = ctx->md_data;
	memset(&(c->dctx),0,sizeof(gost_hash_ctx));
	gost_init(&(c->cctx),&GostR3411_94_CryptoProParamSet);
	c->dctx.cipher_ctx= &(c->cctx);
	return 1;
	}

int gost_digest_update(EVP_MD_CTX *ctx,const void *data,size_t count) 
	{
	return hash_block((gost_hash_ctx *)ctx->md_data,data,count);	
	}

int gost_digest_final(EVP_MD_CTX *ctx,unsigned char *md)
	{
	return finish_hash((gost_hash_ctx *)ctx->md_data,md);
	
	}

int gost_digest_copy(EVP_MD_CTX *to,const EVP_MD_CTX *from) 
	{
	struct ossl_gost_digest_ctx *md_ctx=to->md_data;
	if (to->md_data && from->md_data) {
		memcpy(to->md_data,from->md_data,sizeof(struct ossl_gost_digest_ctx));
		md_ctx->dctx.cipher_ctx=&(md_ctx->cctx);
	}
	return 1;
	}		

int gost_digest_cleanup(EVP_MD_CTX *ctx) 
	{
	if (ctx->md_data)
	memset(ctx->md_data,0,sizeof(struct ossl_gost_digest_ctx));
	return 1;
	}	