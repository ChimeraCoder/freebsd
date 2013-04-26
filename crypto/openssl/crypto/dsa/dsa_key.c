
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
#include <time.h>
#include "cryptlib.h"
#ifndef OPENSSL_NO_SHA
#include <openssl/bn.h>
#include <openssl/dsa.h>
#include <openssl/rand.h>

#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#endif

static int dsa_builtin_keygen(DSA *dsa);

int DSA_generate_key(DSA *dsa)
	{
#ifdef OPENSSL_FIPS
	if (FIPS_mode() && !(dsa->meth->flags & DSA_FLAG_FIPS_METHOD)
			&& !(dsa->flags & DSA_FLAG_NON_FIPS_ALLOW))
		{
		DSAerr(DSA_F_DSA_GENERATE_KEY, DSA_R_NON_FIPS_DSA_METHOD);
		return 0;
		}
#endif
	if(dsa->meth->dsa_keygen)
		return dsa->meth->dsa_keygen(dsa);
#ifdef OPENSSL_FIPS
	if (FIPS_mode())
		return FIPS_dsa_generate_key(dsa);
#endif
	return dsa_builtin_keygen(dsa);
	}

static int dsa_builtin_keygen(DSA *dsa)
	{
	int ok=0;
	BN_CTX *ctx=NULL;
	BIGNUM *pub_key=NULL,*priv_key=NULL;

	if ((ctx=BN_CTX_new()) == NULL) goto err;

	if (dsa->priv_key == NULL)
		{
		if ((priv_key=BN_new()) == NULL) goto err;
		}
	else
		priv_key=dsa->priv_key;

	do
		if (!BN_rand_range(priv_key,dsa->q)) goto err;
	while (BN_is_zero(priv_key));

	if (dsa->pub_key == NULL)
		{
		if ((pub_key=BN_new()) == NULL) goto err;
		}
	else
		pub_key=dsa->pub_key;
	
	{
		BIGNUM local_prk;
		BIGNUM *prk;

		if ((dsa->flags & DSA_FLAG_NO_EXP_CONSTTIME) == 0)
			{
			BN_init(&local_prk);
			prk = &local_prk;
			BN_with_flags(prk, priv_key, BN_FLG_CONSTTIME);
			}
		else
			prk = priv_key;

		if (!BN_mod_exp(pub_key,dsa->g,prk,dsa->p,ctx)) goto err;
	}

	dsa->priv_key=priv_key;
	dsa->pub_key=pub_key;
	ok=1;

err:
	if ((pub_key != NULL) && (dsa->pub_key == NULL)) BN_free(pub_key);
	if ((priv_key != NULL) && (dsa->priv_key == NULL)) BN_free(priv_key);
	if (ctx != NULL) BN_CTX_free(ctx);
	return(ok);
	}
#endif