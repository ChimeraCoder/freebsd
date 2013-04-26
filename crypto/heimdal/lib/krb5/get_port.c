
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

KRB5_LIB_FUNCTION int KRB5_LIB_CALL
krb5_getportbyname (krb5_context context,
		    const char *service,
		    const char *proto,
		    int default_port)
{
    struct servent *sp;

    if ((sp = roken_getservbyname (service, proto)) == NULL) {
#if 0
	krb5_warnx(context, "%s/%s unknown service, using default port %d",
		   service, proto, default_port);
#endif
	return htons(default_port);
    } else
	return sp->s_port;
}