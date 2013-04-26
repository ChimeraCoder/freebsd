
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

#include <openssl/crypto.h>
#include <openssl/opensslconf.h>
#if !defined(OPENSSL_NO_SHA0) && !defined(OPENSSL_NO_SHA)

#undef  SHA_1
#define SHA_0

#include <openssl/opensslv.h>

const char SHA_version[]="SHA" OPENSSL_VERSION_PTEXT;

/* The implementation is in ../md32_common.h */

#include "sha_locl.h"

#endif