
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
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include "evp_locl.h"

static int do_sigver_init(EVP_MD_CTX *ctx, EVP_PKEY_CTX **pctx,
			  const EVP_MD *type, ENGINE *e, EVP_PKEY *pkey,
			  int ver)
	{
	if (ctx->pctx == NULL)
		ctx->pctx = EVP_PKEY_CTX_new(pkey, e);
	if (ctx->pctx == NULL)
		return 0;

	if (type == NULL)
		{
		int def_nid;
		if (EVP_PKEY_get_default_digest_nid(pkey, &def_nid) > 0)
			type = EVP_get_digestbynid(def_nid);
		}

	if (type == NULL)
		{
		EVPerr(EVP_F_DO_SIGVER_INIT, EVP_R_NO_DEFAULT_DIGEST);
		return 0;
		}

	if (ver)
		{
		if (ctx->pctx->pmeth->verifyctx_init)
			{
			if (ctx->pctx->pmeth->verifyctx_init(ctx->pctx, ctx) <=0)
				return 0;
			ctx->pctx->operation = EVP_PKEY_OP_VERIFYCTX;
			}
		else if (EVP_PKEY_verify_init(ctx->pctx) <= 0)
			return 0;
		}
	else
		{
		if (ctx->pctx->pmeth->signctx_init)
			{
			if (ctx->pctx->pmeth->signctx_init(ctx->pctx, ctx) <= 0)
				return 0;
			ctx->pctx->operation = EVP_PKEY_OP_SIGNCTX;
			}
		else if (EVP_PKEY_sign_init(ctx->pctx) <= 0)
			return 0;
		}
	if (EVP_PKEY_CTX_set_signature_md(ctx->pctx, type) <= 0)
		return 0;
	if (pctx)
		*pctx = ctx->pctx;
	if (!EVP_DigestInit_ex(ctx, type, e))
		return 0;
	return 1;
	}

int EVP_DigestSignInit(EVP_MD_CTX *ctx, EVP_PKEY_CTX **pctx,
			const EVP_MD *type, ENGINE *e, EVP_PKEY *pkey)
	{
	return do_sigver_init(ctx, pctx, type, e, pkey, 0);
	}

int EVP_DigestVerifyInit(EVP_MD_CTX *ctx, EVP_PKEY_CTX **pctx,
			const EVP_MD *type, ENGINE *e, EVP_PKEY *pkey)
	{
	return do_sigver_init(ctx, pctx, type, e, pkey, 1);
	}

int EVP_DigestSignFinal(EVP_MD_CTX *ctx, unsigned char *sigret, size_t *siglen)
	{
	int sctx, r = 0;
	if (ctx->pctx->pmeth->signctx)
		sctx = 1;
	else
		sctx = 0;
	if (sigret)
		{
		EVP_MD_CTX tmp_ctx;
		unsigned char md[EVP_MAX_MD_SIZE];
		unsigned int mdlen;
		EVP_MD_CTX_init(&tmp_ctx);
		if (!EVP_MD_CTX_copy_ex(&tmp_ctx,ctx))
		     	return 0;
		if (sctx)
			r = tmp_ctx.pctx->pmeth->signctx(tmp_ctx.pctx,
					sigret, siglen, &tmp_ctx);
		else
			r = EVP_DigestFinal_ex(&tmp_ctx,md,&mdlen);
		EVP_MD_CTX_cleanup(&tmp_ctx);
		if (sctx || !r)
			return r;
		if (EVP_PKEY_sign(ctx->pctx, sigret, siglen, md, mdlen) <= 0)
			return 0;
		}
	else
		{
		if (sctx)
			{
			if (ctx->pctx->pmeth->signctx(ctx->pctx, sigret, siglen, ctx) <= 0)
				return 0;
			}
		else
			{
			int s = EVP_MD_size(ctx->digest);
			if (s < 0 || EVP_PKEY_sign(ctx->pctx, sigret, siglen, NULL, s) <= 0)
				return 0;
			}
		}
	return 1;
	}

int EVP_DigestVerifyFinal(EVP_MD_CTX *ctx, unsigned char *sig, size_t siglen)
	{
	EVP_MD_CTX tmp_ctx;
	unsigned char md[EVP_MAX_MD_SIZE];
	int r;
	unsigned int mdlen;
	int vctx;

	if (ctx->pctx->pmeth->verifyctx)
		vctx = 1;
	else
		vctx = 0;
	EVP_MD_CTX_init(&tmp_ctx);
	if (!EVP_MD_CTX_copy_ex(&tmp_ctx,ctx))
		return -1;	
	if (vctx)
		{
		r = tmp_ctx.pctx->pmeth->verifyctx(tmp_ctx.pctx,
					sig, siglen, &tmp_ctx);
		}
	else
		r = EVP_DigestFinal_ex(&tmp_ctx,md,&mdlen);
	EVP_MD_CTX_cleanup(&tmp_ctx);
	if (vctx || !r)
		return r;
	return EVP_PKEY_verify(ctx->pctx, sig, siglen, md, mdlen);
	}