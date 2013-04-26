
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

#include "mech_locl.h"

GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_export_name(OM_uint32 *minor_status,
    const gss_name_t input_name,
    gss_buffer_t exported_name)
{
	struct _gss_name *name = (struct _gss_name *) input_name;
	struct _gss_mechanism_name *mn;

	_mg_buffer_zero(exported_name);

	/*
	 * If this name already has any attached MNs, export the first
	 * one, otherwise export based on the first mechanism in our
	 * list.
	 */
	mn = HEIM_SLIST_FIRST(&name->gn_mn);
	if (!mn) {
		*minor_status = 0;
		return (GSS_S_NAME_NOT_MN);
	}

	return mn->gmn_mech->gm_export_name(minor_status,
	    mn->gmn_name, exported_name);
}