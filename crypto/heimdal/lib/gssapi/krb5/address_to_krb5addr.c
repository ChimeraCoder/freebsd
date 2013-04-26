
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

#include "gsskrb5_locl.h"

#include <roken.h>

krb5_error_code
_gsskrb5i_address_to_krb5addr(krb5_context context,
			      OM_uint32 gss_addr_type,
			      gss_buffer_desc *gss_addr,
			      int16_t port,
			      krb5_address *address)
{
   int addr_type;
   struct sockaddr sa;
   krb5_socklen_t sa_size = sizeof(sa);
   krb5_error_code problem;

   if (gss_addr == NULL)
      return GSS_S_FAILURE;

   switch (gss_addr_type) {
#ifdef HAVE_IPV6
      case GSS_C_AF_INET6: addr_type = AF_INET6;
                           break;
#endif /* HAVE_IPV6 */

      case GSS_C_AF_INET:  addr_type = AF_INET;
                           break;
      default:
                           return GSS_S_FAILURE;
   }

   problem = krb5_h_addr2sockaddr (context,
				   addr_type,
                                   gss_addr->value,
                                   &sa,
                                   &sa_size,
                                   port);
   if (problem)
      return GSS_S_FAILURE;

   problem = krb5_sockaddr2address (context, &sa, address);

   return problem;
}