
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

OM_uint32
_gss_find_mn(OM_uint32 *minor_status, struct _gss_name *name, gss_OID mech,
	     struct _gss_mechanism_name **output_mn)
{
	OM_uint32 major_status;
	gssapi_mech_interface m;
	struct _gss_mechanism_name *mn;

	*output_mn = NULL;

	HEIM_SLIST_FOREACH(mn, &name->gn_mn, gmn_link) {
		if (gss_oid_equal(mech, mn->gmn_mech_oid))
			break;
	}

	if (!mn) {
		/*
		 * If this name is canonical (i.e. there is only an
		 * MN but it is from a different mech), give up now.
		 */
		if (!name->gn_value.value)
			return GSS_S_BAD_NAME;

		m = __gss_get_mechanism(mech);
		if (!m)
			return (GSS_S_BAD_MECH);

		mn = malloc(sizeof(struct _gss_mechanism_name));
		if (!mn)
			return GSS_S_FAILURE;

		major_status = m->gm_import_name(minor_status,
		    &name->gn_value,
		    (name->gn_type.elements
			? &name->gn_type : GSS_C_NO_OID),
		    &mn->gmn_name);
		if (major_status != GSS_S_COMPLETE) {
			_gss_mg_error(m, major_status, *minor_status);
			free(mn);
			return major_status;
		}

		mn->gmn_mech = m;
		mn->gmn_mech_oid = &m->gm_mech_oid;
		HEIM_SLIST_INSERT_HEAD(&name->gn_mn, mn, gmn_link);
	}
	*output_mn = mn;
	return 0;
}


/*
 * Make a name from an MN.
 */
struct _gss_name *
_gss_make_name(gssapi_mech_interface m, gss_name_t new_mn)
{
	struct _gss_name *name;
	struct _gss_mechanism_name *mn;

	name = malloc(sizeof(struct _gss_name));
	if (!name)
		return (0);
	memset(name, 0, sizeof(struct _gss_name));

	mn = malloc(sizeof(struct _gss_mechanism_name));
	if (!mn) {
		free(name);
		return (0);
	}

	HEIM_SLIST_INIT(&name->gn_mn);
	mn->gmn_mech = m;
	mn->gmn_mech_oid = &m->gm_mech_oid;
	mn->gmn_name = new_mn;
	HEIM_SLIST_INSERT_HEAD(&name->gn_mn, mn, gmn_link);

	return (name);
}