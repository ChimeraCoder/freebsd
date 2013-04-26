
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
gss_compare_name(OM_uint32 *minor_status,
    const gss_name_t name1_arg,
    const gss_name_t name2_arg,
    int *name_equal)
{
	struct _gss_name *name1 = (struct _gss_name *) name1_arg;
	struct _gss_name *name2 = (struct _gss_name *) name2_arg;

	/*
	 * First check the implementation-independant name if both
	 * names have one. Otherwise, try to find common mechanism
	 * names and compare them.
	 */
	if (name1->gn_value.value && name2->gn_value.value) {
		*name_equal = 1;
		if (!gss_oid_equal(&name1->gn_type, &name2->gn_type)) {
			*name_equal = 0;
		} else if (name1->gn_value.length != name2->gn_value.length ||
		    memcmp(name1->gn_value.value, name1->gn_value.value,
			name1->gn_value.length)) {
			*name_equal = 0;
		}
	} else {
		struct _gss_mechanism_name *mn1;
		struct _gss_mechanism_name *mn2;

		HEIM_SLIST_FOREACH(mn1, &name1->gn_mn, gmn_link) {
			OM_uint32 major_status;

			major_status = _gss_find_mn(minor_status, name2,
						    mn1->gmn_mech_oid, &mn2);
			if (major_status == GSS_S_COMPLETE) {
				return (mn1->gmn_mech->gm_compare_name(
						minor_status,
						mn1->gmn_name,
						mn2->gmn_name,
						name_equal));
			}
		}
		*name_equal = 0;
	}

	*minor_status = 0;
	return (GSS_S_COMPLETE);
}