
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

#include "ssl_locl.h"
#ifndef OPENSSL_NO_SSL2
#include <stdio.h>
#include <openssl/objects.h>

static const SSL_METHOD *ssl2_get_method(int ver);
static const SSL_METHOD *ssl2_get_method(int ver)
	{
	if (ver == SSL2_VERSION)
		return(SSLv2_method());
	else
		return(NULL);
	}

IMPLEMENT_ssl2_meth_func(SSLv2_method,
			 ssl2_accept,
			 ssl2_connect,
			 ssl2_get_method)

#else /* !OPENSSL_NO_SSL2 */

# if PEDANTIC
static void *dummy=&dummy;
# endif

#endif