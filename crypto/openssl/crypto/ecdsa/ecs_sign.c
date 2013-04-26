
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

#include "ecs_locl.h"
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
#include <openssl/rand.h>

ECDSA_SIG *ECDSA_do_sign(const unsigned char *dgst, int dlen, EC_KEY *eckey)
{
	return ECDSA_do_sign_ex(dgst, dlen, NULL, NULL, eckey);
}

ECDSA_SIG *ECDSA_do_sign_ex(const unsigned char *dgst, int dlen,
	const BIGNUM *kinv, const BIGNUM *rp, EC_KEY *eckey)
{
	ECDSA_DATA *ecdsa = ecdsa_check(eckey);
	if (ecdsa == NULL)
		return NULL;
	return ecdsa->meth->ecdsa_do_sign(dgst, dlen, kinv, rp, eckey);
}

int ECDSA_sign(int type, const unsigned char *dgst, int dlen, unsigned char 
		*sig, unsigned int *siglen, EC_KEY *eckey)
{
	return ECDSA_sign_ex(type, dgst, dlen, sig, siglen, NULL, NULL, eckey);
}

int ECDSA_sign_ex(int type, const unsigned char *dgst, int dlen, unsigned char 
	*sig, unsigned int *siglen, const BIGNUM *kinv, const BIGNUM *r, 
	EC_KEY *eckey)
{
	ECDSA_SIG *s;
	RAND_seed(dgst, dlen);
	s = ECDSA_do_sign_ex(dgst, dlen, kinv, r, eckey);
	if (s == NULL)
	{
		*siglen=0;
		return 0;
	}
	*siglen = i2d_ECDSA_SIG(s, &sig);
	ECDSA_SIG_free(s);
	return 1;
}

int ECDSA_sign_setup(EC_KEY *eckey, BN_CTX *ctx_in, BIGNUM **kinvp, 
		BIGNUM **rp)
{
	ECDSA_DATA *ecdsa = ecdsa_check(eckey);
	if (ecdsa == NULL)
		return 0;
	return ecdsa->meth->ecdsa_sign_setup(eckey, ctx_in, kinvp, rp); 
}