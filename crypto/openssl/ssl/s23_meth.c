
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

static const SSL_METHOD *ssl23_get_method(int ver);
static const SSL_METHOD *ssl23_get_method(int ver)
	{
#ifndef OPENSSL_NO_SSL2
	if (ver == SSL2_VERSION)
		return(SSLv2_method());
	else
#endif
#ifndef OPENSSL_NO_SSL3
	if (ver == SSL3_VERSION)
		return(SSLv3_method());
	else
#endif
#ifndef OPENSSL_NO_TLS1
	if (ver == TLS1_VERSION)
		return(TLSv1_method());
	else if (ver == TLS1_1_VERSION)
		return(TLSv1_1_method());
	else if (ver == TLS1_2_VERSION)
		return(TLSv1_2_method());
	else
#endif
		return(NULL);
	}

IMPLEMENT_ssl23_meth_func(SSLv23_method,
			ssl23_accept,
			ssl23_connect,
			ssl23_get_method)