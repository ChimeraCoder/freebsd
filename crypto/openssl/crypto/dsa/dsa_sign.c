
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

/* Original version from Steven Schoch <schoch@sheba.arc.nasa.gov> */

#include "cryptlib.h"
#include <openssl/dsa.h>
#include <openssl/rand.h>
#include <openssl/bn.h>

DSA_SIG * DSA_do_sign(const unsigned char *dgst, int dlen, DSA *dsa)
	{
#ifdef OPENSSL_FIPS
	if (FIPS_mode() && !(dsa->meth->flags & DSA_FLAG_FIPS_METHOD)
			&& !(dsa->flags & DSA_FLAG_NON_FIPS_ALLOW))
		{
		DSAerr(DSA_F_DSA_DO_SIGN, DSA_R_NON_FIPS_DSA_METHOD);
		return NULL;
		}
#endif
	return dsa->meth->dsa_do_sign(dgst, dlen, dsa);
	}

int DSA_sign_setup(DSA *dsa, BN_CTX *ctx_in, BIGNUM **kinvp, BIGNUM **rp)
	{
#ifdef OPENSSL_FIPS
	if (FIPS_mode() && !(dsa->meth->flags & DSA_FLAG_FIPS_METHOD)
			&& !(dsa->flags & DSA_FLAG_NON_FIPS_ALLOW))
		{
		DSAerr(DSA_F_DSA_SIGN_SETUP, DSA_R_NON_FIPS_DSA_METHOD);
		return 0;
		}
#endif
	return dsa->meth->dsa_sign_setup(dsa, ctx_in, kinvp, rp);
	}

DSA_SIG *DSA_SIG_new(void)
	{
	DSA_SIG *sig;
	sig = OPENSSL_malloc(sizeof(DSA_SIG));
	if (!sig)
		return NULL;
	sig->r = NULL;
	sig->s = NULL;
	return sig;
	}

void DSA_SIG_free(DSA_SIG *sig)
	{
	if (sig)
		{
		if (sig->r)
			BN_free(sig->r);
		if (sig->s)
			BN_free(sig->s);
		OPENSSL_free(sig);
		}
	}