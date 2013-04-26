
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
gss_duplicate_name(OM_uint32 *minor_status,
    const gss_name_t src_name,
    gss_name_t *dest_name)
{
	OM_uint32		major_status;
	struct _gss_name	*name = (struct _gss_name *) src_name;
	struct _gss_name	*new_name;
	struct _gss_mechanism_name *mn;

	*minor_status = 0;
	*dest_name = GSS_C_NO_NAME;

	/*
	 * If this name has a value (i.e. it didn't come from
	 * gss_canonicalize_name(), we re-import the thing. Otherwise,
	 * we make copy of each mech names.
	 */
	if (name->gn_value.value) {
		major_status = gss_import_name(minor_status,
		    &name->gn_value, &name->gn_type, dest_name);
		if (major_status != GSS_S_COMPLETE)
			return (major_status);
		new_name = (struct _gss_name *) *dest_name;

		HEIM_SLIST_FOREACH(mn, &name->gn_mn, gmn_link) {
		    struct _gss_mechanism_name *mn2;
		    _gss_find_mn(minor_status, new_name,
				 mn->gmn_mech_oid, &mn2);
		}
	} else {
		new_name = malloc(sizeof(struct _gss_name));
		if (!new_name) {
			*minor_status = ENOMEM;
			return (GSS_S_FAILURE);
		}
		memset(new_name, 0, sizeof(struct _gss_name));
		HEIM_SLIST_INIT(&new_name->gn_mn);
		*dest_name = (gss_name_t) new_name;

		HEIM_SLIST_FOREACH(mn, &name->gn_mn, gmn_link) {
			struct _gss_mechanism_name *new_mn;

			new_mn = malloc(sizeof(*new_mn));
			if (!new_mn) {
				*minor_status = ENOMEM;
				return GSS_S_FAILURE;
			}
			new_mn->gmn_mech = mn->gmn_mech;
			new_mn->gmn_mech_oid = mn->gmn_mech_oid;

			major_status =
			    mn->gmn_mech->gm_duplicate_name(minor_status,
				mn->gmn_name, &new_mn->gmn_name);
			if (major_status != GSS_S_COMPLETE) {
				free(new_mn);
				continue;
			}
			HEIM_SLIST_INSERT_HEAD(&new_name->gn_mn, new_mn, gmn_link);
		}

	}

	return (GSS_S_COMPLETE);
}