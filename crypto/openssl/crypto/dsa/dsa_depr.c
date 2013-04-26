
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

/* This file contains deprecated function(s) that are now wrappers to the new
 * version(s). */

#undef GENUINE_DSA

#ifdef GENUINE_DSA
/* Parameter generation follows the original release of FIPS PUB 186,
 * Appendix 2.2 (i.e. use SHA as defined in FIPS PUB 180) */
#define HASH    EVP_sha()
#else
/* Parameter generation follows the updated Appendix 2.2 for FIPS PUB 186,
 * also Appendix 2.2 of FIPS PUB 186-1 (i.e. use SHA as defined in
 * FIPS PUB 180-1) */
#define HASH    EVP_sha1()
#endif 

static void *dummy=&dummy;

#ifndef OPENSSL_NO_SHA

#include <stdio.h>
#include <time.h>
#include "cryptlib.h"
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/dsa.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#ifndef OPENSSL_NO_DEPRECATED
DSA *DSA_generate_parameters(int bits,
		unsigned char *seed_in, int seed_len,
		int *counter_ret, unsigned long *h_ret,
		void (*callback)(int, int, void *),
		void *cb_arg)
	{
	BN_GENCB cb;
	DSA *ret;

	if ((ret=DSA_new()) == NULL) return NULL;

	BN_GENCB_set_old(&cb, callback, cb_arg);

	if(DSA_generate_parameters_ex(ret, bits, seed_in, seed_len,
				counter_ret, h_ret, &cb))
		return ret;
	DSA_free(ret);
	return NULL;
	}
#endif
#endif