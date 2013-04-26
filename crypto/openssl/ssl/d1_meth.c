
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

static const SSL_METHOD *dtls1_get_method(int ver);
static const SSL_METHOD *dtls1_get_method(int ver)
	{
	if (ver == DTLS1_VERSION)
		return(DTLSv1_method());
	else
		return(NULL);
	}

IMPLEMENT_dtls1_meth_func(DTLSv1_method,
			dtls1_accept,
			dtls1_connect,
			dtls1_get_method)