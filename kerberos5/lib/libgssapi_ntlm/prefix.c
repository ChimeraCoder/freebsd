
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

#include <gssapi/gssapi.h>

gss_OID_desc __gss_c_nt_user_name_oid_desc =
    {10, (void *)("\x2a\x86\x48\x86\xf7\x12" "\x01\x02\x01\x01")};
gss_OID_desc __gss_c_nt_hostbased_service_oid_desc =
    {10, (void *)("\x2a\x86\x48\x86\xf7\x12" "\x01\x02\x01\x04")};

const char *
_gss_name_prefix(void)
{
	return "_gss_ntlm";
}