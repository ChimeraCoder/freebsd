
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
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#if 0
#undef OpenSSL_add_all_algorithms

void OpenSSL_add_all_algorithms(void)
	{
	OPENSSL_add_all_algorithms_noconf();
	}
#endif

void OPENSSL_add_all_algorithms_noconf(void)
	{
	/*
	 * For the moment OPENSSL_cpuid_setup does something
	 * only on IA-32, but we reserve the option for all
	 * platforms...
	 */
	OPENSSL_cpuid_setup();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
#ifndef OPENSSL_NO_ENGINE
# if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(HAVE_CRYPTODEV)
	ENGINE_setup_bsd_cryptodev();
# endif
#endif
	}