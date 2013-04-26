
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

#include "krb5_locl.h"

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_sock_to_principal (krb5_context context,
			int sock,
			const char *sname,
			int32_t type,
			krb5_principal *ret_princ)
{
    krb5_error_code ret;
    struct sockaddr_storage __ss;
    struct sockaddr *sa = (struct sockaddr *)&__ss;
    socklen_t salen = sizeof(__ss);
    char hostname[NI_MAXHOST];

    if (getsockname (sock, sa, &salen) < 0) {
	ret = errno;
	krb5_set_error_message (context, ret, "getsockname: %s", strerror(ret));
	return ret;
    }
    ret = getnameinfo (sa, salen, hostname, sizeof(hostname), NULL, 0, 0);
    if (ret) {
	int save_errno = errno;
	krb5_error_code ret2 = krb5_eai_to_heim_errno(ret, save_errno);
	krb5_set_error_message (context, ret2, "getnameinfo: %s", gai_strerror(ret));
	return ret2;
    }

    ret = krb5_sname_to_principal (context,
				   hostname,
				   sname,
				   type,
				   ret_princ);
    return ret;
}