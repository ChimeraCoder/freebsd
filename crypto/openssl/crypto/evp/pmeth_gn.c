
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
#include "cryptlib.h"
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include "evp_locl.h"

int EVP_PKEY_paramgen_init(EVP_PKEY_CTX *ctx)
	{
	int ret;
	if (!ctx || !ctx->pmeth || !ctx->pmeth->paramgen)
		{
		EVPerr(EVP_F_EVP_PKEY_PARAMGEN_INIT,
			EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
		return -2;
		}
	ctx->operation = EVP_PKEY_OP_PARAMGEN;
	if (!ctx->pmeth->paramgen_init)
		return 1;
	ret = ctx->pmeth->paramgen_init(ctx);
	if (ret <= 0)
		ctx->operation = EVP_PKEY_OP_UNDEFINED;
	return ret;
	}

int EVP_PKEY_paramgen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey)
	{
	int ret;
	if (!ctx || !ctx->pmeth || !ctx->pmeth->paramgen)
		{
		EVPerr(EVP_F_EVP_PKEY_PARAMGEN,
			EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
		return -2;
		}

	if (ctx->operation != EVP_PKEY_OP_PARAMGEN)
		{
		EVPerr(EVP_F_EVP_PKEY_PARAMGEN, EVP_R_OPERATON_NOT_INITIALIZED);
		return -1;
		}

	if (!ppkey)
		return -1;

	if (!*ppkey)
		*ppkey = EVP_PKEY_new();

	ret = ctx->pmeth->paramgen(ctx, *ppkey);
	if (ret <= 0)
		{
		EVP_PKEY_free(*ppkey);
		*ppkey = NULL;
		}
	return ret;
	}

int EVP_PKEY_keygen_init(EVP_PKEY_CTX *ctx)
	{
	int ret;
	if (!ctx || !ctx->pmeth || !ctx->pmeth->keygen)
		{
		EVPerr(EVP_F_EVP_PKEY_KEYGEN_INIT,
			EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
		return -2;
		}
	ctx->operation = EVP_PKEY_OP_KEYGEN;
	if (!ctx->pmeth->keygen_init)
		return 1;
	ret = ctx->pmeth->keygen_init(ctx);
	if (ret <= 0)
		ctx->operation = EVP_PKEY_OP_UNDEFINED;
	return ret;
	}

int EVP_PKEY_keygen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey)
	{
	int ret;

	if (!ctx || !ctx->pmeth || !ctx->pmeth->keygen)
		{
		EVPerr(EVP_F_EVP_PKEY_KEYGEN,
			EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
		return -2;
		}
	if (ctx->operation != EVP_PKEY_OP_KEYGEN)
		{
		EVPerr(EVP_F_EVP_PKEY_KEYGEN, EVP_R_OPERATON_NOT_INITIALIZED);
		return -1;
		}

	if (!ppkey)
		return -1;

	if (!*ppkey)
		*ppkey = EVP_PKEY_new();

	ret = ctx->pmeth->keygen(ctx, *ppkey);
	if (ret <= 0)
		{
		EVP_PKEY_free(*ppkey);
		*ppkey = NULL;
		}
	return ret;
	}

void EVP_PKEY_CTX_set_cb(EVP_PKEY_CTX *ctx, EVP_PKEY_gen_cb *cb)
	{
	ctx->pkey_gencb = cb;
	}

EVP_PKEY_gen_cb *EVP_PKEY_CTX_get_cb(EVP_PKEY_CTX *ctx)
	{
	return ctx->pkey_gencb;
	}

/* "translation callback" to call EVP_PKEY_CTX callbacks using BN_GENCB
 * style callbacks.
 */

static int trans_cb(int a, int b, BN_GENCB *gcb)
	{
	EVP_PKEY_CTX *ctx = gcb->arg;
	ctx->keygen_info[0] = a;
	ctx->keygen_info[1] = b;
	return ctx->pkey_gencb(ctx);
	}	

void evp_pkey_set_cb_translate(BN_GENCB *cb, EVP_PKEY_CTX *ctx)
	{
	BN_GENCB_set(cb, trans_cb, ctx)
	}

int EVP_PKEY_CTX_get_keygen_info(EVP_PKEY_CTX *ctx, int idx)
	{
	if (idx == -1)
		return ctx->keygen_info_count; 
	if (idx < 0 || idx > ctx->keygen_info_count)
		return 0;
	return ctx->keygen_info[idx];
	}

EVP_PKEY *EVP_PKEY_new_mac_key(int type, ENGINE *e,
				const unsigned char *key, int keylen)
	{
	EVP_PKEY_CTX *mac_ctx = NULL;
	EVP_PKEY *mac_key = NULL;
	mac_ctx = EVP_PKEY_CTX_new_id(type, e);
	if (!mac_ctx)
		return NULL;
	if (EVP_PKEY_keygen_init(mac_ctx) <= 0)
		goto merr;
	if (EVP_PKEY_CTX_ctrl(mac_ctx, -1, EVP_PKEY_OP_KEYGEN,
				EVP_PKEY_CTRL_SET_MAC_KEY,
				keylen, (void *)key) <= 0)
		goto merr;
	if (EVP_PKEY_keygen(mac_ctx, &mac_key) <= 0)
		goto merr;
	merr:
	if (mac_ctx)
		EVP_PKEY_CTX_free(mac_ctx);
	return mac_key;
	}