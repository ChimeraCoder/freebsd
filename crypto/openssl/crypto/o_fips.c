
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

#include "cryptlib.h"
#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#include <openssl/fips_rand.h>
#include <openssl/rand.h>
#endif

int FIPS_mode(void)
	{
	OPENSSL_init();
#ifdef OPENSSL_FIPS
	return FIPS_module_mode();
#else
	return 0;
#endif
	}

int FIPS_mode_set(int r)
	{
	OPENSSL_init();
#ifdef OPENSSL_FIPS
#ifndef FIPS_AUTH_USER_PASS
#define FIPS_AUTH_USER_PASS	"Default FIPS Crypto User Password"
#endif
	if (!FIPS_module_mode_set(r, FIPS_AUTH_USER_PASS))
		return 0;
	if (r)
		RAND_set_rand_method(FIPS_rand_get_method());
	else
		RAND_set_rand_method(NULL);
	return 1;
#else
	if (r == 0)
		return 1;
	CRYPTOerr(CRYPTO_F_FIPS_MODE_SET, CRYPTO_R_FIPS_MODE_NOT_SUPPORTED);
	return 0;
#endif
	}