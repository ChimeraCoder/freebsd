
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
#include <openssl/objects.h>
#include "ssl_locl.h"

static const SSL_METHOD *tls1_get_method(int ver)
	{
	if (ver == TLS1_2_VERSION)
		return TLSv1_2_method();
	if (ver == TLS1_1_VERSION)
		return TLSv1_1_method();
	if (ver == TLS1_VERSION)
		return TLSv1_method();
	return NULL;
	}

IMPLEMENT_tls_meth_func(TLS1_2_VERSION, TLSv1_2_method,
			ssl3_accept,
			ssl3_connect,
			tls1_get_method)

IMPLEMENT_tls_meth_func(TLS1_1_VERSION, TLSv1_1_method,
			ssl3_accept,
			ssl3_connect,
			tls1_get_method)

IMPLEMENT_tls_meth_func(TLS1_VERSION, TLSv1_method,
			ssl3_accept,
			ssl3_connect,
			tls1_get_method)