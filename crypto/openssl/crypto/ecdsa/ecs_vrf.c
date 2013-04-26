
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

/* returns
 *      1: correct signature
 *      0: incorrect signature
 *     -1: error
 */
int ECDSA_do_verify(const unsigned char *dgst, int dgst_len, 
		const ECDSA_SIG *sig, EC_KEY *eckey)
	{
	ECDSA_DATA *ecdsa = ecdsa_check(eckey);
	if (ecdsa == NULL)
		return 0;
	return ecdsa->meth->ecdsa_do_verify(dgst, dgst_len, sig, eckey);
	}

/* returns
 *      1: correct signature
 *      0: incorrect signature
 *     -1: error
 */
int ECDSA_verify(int type, const unsigned char *dgst, int dgst_len,
		const unsigned char *sigbuf, int sig_len, EC_KEY *eckey)
 	{
	ECDSA_SIG *s;
	int ret=-1;

	s = ECDSA_SIG_new();
	if (s == NULL) return(ret);
	if (d2i_ECDSA_SIG(&s, &sigbuf, sig_len) == NULL) goto err;
	ret=ECDSA_do_verify(dgst, dgst_len, s, eckey);
err:
	ECDSA_SIG_free(s);
	return(ret);
	}