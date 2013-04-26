
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

/**
 *  gss_canonicalize_name takes a Internal Name (IN) and converts in into a
 *  mechanism specific Mechanism Name (MN).
 *
 *  The input name may multiple name, or generic name types.
 *
 *  If the input_name if of the GSS_C_NT_USER_NAME, and the Kerberos
 *  mechanism is specified, the resulting MN type is a
 *  GSS_KRB5_NT_PRINCIPAL_NAME.
 *
 *  For more information about @ref internalVSmechname.
 *
 *  @param minor_status minor status code.
 *  @param input_name name to covert, unchanged by gss_canonicalize_name().
 *  @param mech_type the type to convert Name too.
 *  @param output_name the resulting type, release with
 *         gss_release_name(), independent of input_name.
 *
 *  @returns a gss_error code, see gss_display_status() about printing
 *         the error code.
 *
 *  @ingroup gssapi
 */

GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_canonicalize_name(OM_uint32 *minor_status,
    const gss_name_t input_name,
    const gss_OID mech_type,
    gss_name_t *output_name)
{
	OM_uint32 major_status;
	struct _gss_name *name = (struct _gss_name *) input_name;
	struct _gss_mechanism_name *mn;
	gssapi_mech_interface m;
	gss_name_t new_canonical_name;

	*minor_status = 0;
	*output_name = 0;

	major_status = _gss_find_mn(minor_status, name, mech_type, &mn);
	if (major_status)
		return major_status;

	m = mn->gmn_mech;
	major_status = m->gm_canonicalize_name(minor_status,
	    mn->gmn_name, mech_type, &new_canonical_name);
	if (major_status) {
		_gss_mg_error(m, major_status, *minor_status);
		return (major_status);
	}

	/*
	 * Now we make a new name and mark it as an MN.
	 */
	*minor_status = 0;
	name = malloc(sizeof(struct _gss_name));
	if (!name) {
		m->gm_release_name(minor_status, &new_canonical_name);
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}
	memset(name, 0, sizeof(struct _gss_name));

	mn = malloc(sizeof(struct _gss_mechanism_name));
	if (!mn) {
		m->gm_release_name(minor_status, &new_canonical_name);
		free(name);
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}

	HEIM_SLIST_INIT(&name->gn_mn);
	mn->gmn_mech = m;
	mn->gmn_mech_oid = &m->gm_mech_oid;
	mn->gmn_name = new_canonical_name;
	HEIM_SLIST_INSERT_HEAD(&name->gn_mn, mn, gmn_link);

	*output_name = (gss_name_t) name;

	return (GSS_S_COMPLETE);
}