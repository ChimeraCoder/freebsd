
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
#include <openssl/objects.h>
#include <openssl/evp.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#endif

void EVP_MD_CTX_init(EVP_MD_CTX *ctx)
	{
	memset(ctx,'\0',sizeof *ctx);
	}

EVP_MD_CTX *EVP_MD_CTX_create(void)
	{
	EVP_MD_CTX *ctx=OPENSSL_malloc(sizeof *ctx);

	if (ctx)
		EVP_MD_CTX_init(ctx);

	return ctx;
	}

int EVP_DigestInit(EVP_MD_CTX *ctx, const EVP_MD *type)
	{
	EVP_MD_CTX_init(ctx);
	return EVP_DigestInit_ex(ctx, type, NULL);
	}

int EVP_DigestInit_ex(EVP_MD_CTX *ctx, const EVP_MD *type, ENGINE *impl)
	{
	EVP_MD_CTX_clear_flags(ctx,EVP_MD_CTX_FLAG_CLEANED);
#ifndef OPENSSL_NO_ENGINE
	/* Whether it's nice or not, "Inits" can be used on "Final"'d contexts
	 * so this context may already have an ENGINE! Try to avoid releasing
	 * the previous handle, re-querying for an ENGINE, and having a
	 * reinitialisation, when it may all be unecessary. */
	if (ctx->engine && ctx->digest && (!type ||
			(type && (type->type == ctx->digest->type))))
		goto skip_to_init;
	if (type)
		{
		/* Ensure an ENGINE left lying around from last time is cleared
		 * (the previous check attempted to avoid this if the same
		 * ENGINE and EVP_MD could be used). */
		if(ctx->engine)
			ENGINE_finish(ctx->engine);
		if(impl)
			{
			if (!ENGINE_init(impl))
				{
				EVPerr(EVP_F_EVP_DIGESTINIT_EX,EVP_R_INITIALIZATION_ERROR);
				return 0;
				}
			}
		else
			/* Ask if an ENGINE is reserved for this job */
			impl = ENGINE_get_digest_engine(type->type);
		if(impl)
			{
			/* There's an ENGINE for this job ... (apparently) */
			const EVP_MD *d = ENGINE_get_digest(impl, type->type);
			if(!d)
				{
				/* Same comment from evp_enc.c */
				EVPerr(EVP_F_EVP_DIGESTINIT_EX,EVP_R_INITIALIZATION_ERROR);
				ENGINE_finish(impl);
				return 0;
				}
			/* We'll use the ENGINE's private digest definition */
			type = d;
			/* Store the ENGINE functional reference so we know
			 * 'type' came from an ENGINE and we need to release
			 * it when done. */
			ctx->engine = impl;
			}
		else
			ctx->engine = NULL;
		}
	else
	if(!ctx->digest)
		{
		EVPerr(EVP_F_EVP_DIGESTINIT_EX,EVP_R_NO_DIGEST_SET);
		return 0;
		}
#endif
	if (ctx->digest != type)
		{
		if (ctx->digest && ctx->digest->ctx_size)
			OPENSSL_free(ctx->md_data);
		ctx->digest=type;
		if (!(ctx->flags & EVP_MD_CTX_FLAG_NO_INIT) && type->ctx_size)
			{
			ctx->update = type->update;
			ctx->md_data=OPENSSL_malloc(type->ctx_size);
			if (ctx->md_data == NULL)
				{
				EVPerr(EVP_F_EVP_DIGESTINIT_EX,
							ERR_R_MALLOC_FAILURE);
				return 0;
				}
			}
		}
#ifndef OPENSSL_NO_ENGINE
skip_to_init:
#endif
	if (ctx->pctx)
		{
		int r;
		r = EVP_PKEY_CTX_ctrl(ctx->pctx, -1, EVP_PKEY_OP_TYPE_SIG,
					EVP_PKEY_CTRL_DIGESTINIT, 0, ctx);
		if (r <= 0 && (r != -2))
			return 0;
		}
	if (ctx->flags & EVP_MD_CTX_FLAG_NO_INIT)
		return 1;
#ifdef OPENSSL_FIPS
	if (FIPS_mode())
		{
		if (FIPS_digestinit(ctx, type))
			return 1;
		OPENSSL_free(ctx->md_data);
		ctx->md_data = NULL;
		return 0;
		}
#endif
	return ctx->digest->init(ctx);
	}

int EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *data, size_t count)
	{
#ifdef OPENSSL_FIPS
	return FIPS_digestupdate(ctx, data, count);
#else
	return ctx->update(ctx,data,count);
#endif
	}

/* The caller can assume that this removes any secret data from the context */
int EVP_DigestFinal(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *size)
	{
	int ret;
	ret = EVP_DigestFinal_ex(ctx, md, size);
	EVP_MD_CTX_cleanup(ctx);
	return ret;
	}

/* The caller can assume that this removes any secret data from the context */
int EVP_DigestFinal_ex(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *size)
	{
#ifdef OPENSSL_FIPS
	return FIPS_digestfinal(ctx, md, size);
#else
	int ret;

	OPENSSL_assert(ctx->digest->md_size <= EVP_MAX_MD_SIZE);
	ret=ctx->digest->final(ctx,md);
	if (size != NULL)
		*size=ctx->digest->md_size;
	if (ctx->digest->cleanup)
		{
		ctx->digest->cleanup(ctx);
		EVP_MD_CTX_set_flags(ctx,EVP_MD_CTX_FLAG_CLEANED);
		}
	memset(ctx->md_data,0,ctx->digest->ctx_size);
	return ret;
#endif
	}

int EVP_MD_CTX_copy(EVP_MD_CTX *out, const EVP_MD_CTX *in)
	{
	EVP_MD_CTX_init(out);
	return EVP_MD_CTX_copy_ex(out, in);
	}

int EVP_MD_CTX_copy_ex(EVP_MD_CTX *out, const EVP_MD_CTX *in)
	{
	unsigned char *tmp_buf;
	if ((in == NULL) || (in->digest == NULL))
		{
		EVPerr(EVP_F_EVP_MD_CTX_COPY_EX,EVP_R_INPUT_NOT_INITIALIZED);
		return 0;
		}
#ifndef OPENSSL_NO_ENGINE
	/* Make sure it's safe to copy a digest context using an ENGINE */
	if (in->engine && !ENGINE_init(in->engine))
		{
		EVPerr(EVP_F_EVP_MD_CTX_COPY_EX,ERR_R_ENGINE_LIB);
		return 0;
		}
#endif

	if (out->digest == in->digest)
		{
		tmp_buf = out->md_data;
	    	EVP_MD_CTX_set_flags(out,EVP_MD_CTX_FLAG_REUSE);
		}
	else tmp_buf = NULL;
	EVP_MD_CTX_cleanup(out);
	memcpy(out,in,sizeof *out);

	if (in->md_data && out->digest->ctx_size)
		{
		if (tmp_buf)
			out->md_data = tmp_buf;
		else
			{
			out->md_data=OPENSSL_malloc(out->digest->ctx_size);
			if (!out->md_data)
				{
				EVPerr(EVP_F_EVP_MD_CTX_COPY_EX,ERR_R_MALLOC_FAILURE);
				return 0;
				}
			}
		memcpy(out->md_data,in->md_data,out->digest->ctx_size);
		}

	out->update = in->update;

	if (in->pctx)
		{
		out->pctx = EVP_PKEY_CTX_dup(in->pctx);
		if (!out->pctx)
			{
			EVP_MD_CTX_cleanup(out);
			return 0;
			}
		}

	if (out->digest->copy)
		return out->digest->copy(out,in);
	
	return 1;
	}

int EVP_Digest(const void *data, size_t count,
		unsigned char *md, unsigned int *size, const EVP_MD *type, ENGINE *impl)
	{
	EVP_MD_CTX ctx;
	int ret;

	EVP_MD_CTX_init(&ctx);
	EVP_MD_CTX_set_flags(&ctx,EVP_MD_CTX_FLAG_ONESHOT);
	ret=EVP_DigestInit_ex(&ctx, type, impl)
	  && EVP_DigestUpdate(&ctx, data, count)
	  && EVP_DigestFinal_ex(&ctx, md, size);
	EVP_MD_CTX_cleanup(&ctx);

	return ret;
	}

void EVP_MD_CTX_destroy(EVP_MD_CTX *ctx)
	{
	EVP_MD_CTX_cleanup(ctx);
	OPENSSL_free(ctx);
	}

/* This call frees resources associated with the context */
int EVP_MD_CTX_cleanup(EVP_MD_CTX *ctx)
	{
#ifndef OPENSSL_FIPS
	/* Don't assume ctx->md_data was cleaned in EVP_Digest_Final,
	 * because sometimes only copies of the context are ever finalised.
	 */
	if (ctx->digest && ctx->digest->cleanup
	    && !EVP_MD_CTX_test_flags(ctx,EVP_MD_CTX_FLAG_CLEANED))
		ctx->digest->cleanup(ctx);
	if (ctx->digest && ctx->digest->ctx_size && ctx->md_data
	    && !EVP_MD_CTX_test_flags(ctx, EVP_MD_CTX_FLAG_REUSE))
		{
		OPENSSL_cleanse(ctx->md_data,ctx->digest->ctx_size);
		OPENSSL_free(ctx->md_data);
		}
#endif
	if (ctx->pctx)
		EVP_PKEY_CTX_free(ctx->pctx);
#ifndef OPENSSL_NO_ENGINE
	if(ctx->engine)
		/* The EVP_MD we used belongs to an ENGINE, release the
		 * functional reference we held for this reason. */
		ENGINE_finish(ctx->engine);
#endif
#ifdef OPENSSL_FIPS
	FIPS_md_ctx_cleanup(ctx);
#endif
	memset(ctx,'\0',sizeof *ctx);

	return 1;
	}