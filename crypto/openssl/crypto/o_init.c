
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

#include <e_os.h>
#include <openssl/err.h>
#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#include <openssl/rand.h>
#endif

/* Perform any essential OpenSSL initialization operations.
 * Currently only sets FIPS callbacks
 */

void OPENSSL_init(void)
	{
	static int done = 0;
	if (done)
		return;
	done = 1;
#ifdef OPENSSL_FIPS
	FIPS_set_locking_callbacks(CRYPTO_lock, CRYPTO_add_lock);
	FIPS_set_error_callbacks(ERR_put_error, ERR_add_error_vdata);
	FIPS_set_malloc_callbacks(CRYPTO_malloc, CRYPTO_free);
	RAND_init_fips();
#endif
#if 0
	fprintf(stderr, "Called OPENSSL_init\n");
#endif
	}