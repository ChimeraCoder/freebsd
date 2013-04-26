
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
#include <openssl/bn.h>
#include <openssl/dh.h>

/* Check that p is a safe prime and
 * if g is 2, 3 or 5, check that it is a suitable generator
 * where
 * for 2, p mod 24 == 11
 * for 3, p mod 12 == 5
 * for 5, p mod 10 == 3 or 7
 * should hold.
 */

int DH_check(const DH *dh, int *ret)
	{
	int ok=0;
	BN_CTX *ctx=NULL;
	BN_ULONG l;
	BIGNUM *q=NULL;

	*ret=0;
	ctx=BN_CTX_new();
	if (ctx == NULL) goto err;
	q=BN_new();
	if (q == NULL) goto err;

	if (BN_is_word(dh->g,DH_GENERATOR_2))
		{
		l=BN_mod_word(dh->p,24);
		if (l != 11) *ret|=DH_NOT_SUITABLE_GENERATOR;
		}
#if 0
	else if (BN_is_word(dh->g,DH_GENERATOR_3))
		{
		l=BN_mod_word(dh->p,12);
		if (l != 5) *ret|=DH_NOT_SUITABLE_GENERATOR;
		}
#endif
	else if (BN_is_word(dh->g,DH_GENERATOR_5))
		{
		l=BN_mod_word(dh->p,10);
		if ((l != 3) && (l != 7))
			*ret|=DH_NOT_SUITABLE_GENERATOR;
		}
	else
		*ret|=DH_UNABLE_TO_CHECK_GENERATOR;

	if (!BN_is_prime_ex(dh->p,BN_prime_checks,ctx,NULL))
		*ret|=DH_CHECK_P_NOT_PRIME;
	else
		{
		if (!BN_rshift1(q,dh->p)) goto err;
		if (!BN_is_prime_ex(q,BN_prime_checks,ctx,NULL))
			*ret|=DH_CHECK_P_NOT_SAFE_PRIME;
		}
	ok=1;
err:
	if (ctx != NULL) BN_CTX_free(ctx);
	if (q != NULL) BN_free(q);
	return(ok);
	}

int DH_check_pub_key(const DH *dh, const BIGNUM *pub_key, int *ret)
	{
	int ok=0;
	BIGNUM *q=NULL;

	*ret=0;
	q=BN_new();
	if (q == NULL) goto err;
	BN_set_word(q,1);
	if (BN_cmp(pub_key,q)<=0)
		*ret|=DH_CHECK_PUBKEY_TOO_SMALL;
	BN_copy(q,dh->p);
	BN_sub_word(q,1);
	if (BN_cmp(pub_key,q)>=0)
		*ret|=DH_CHECK_PUBKEY_TOO_LARGE;

	ok = 1;
err:
	if (q != NULL) BN_free(q);
	return(ok);
	}