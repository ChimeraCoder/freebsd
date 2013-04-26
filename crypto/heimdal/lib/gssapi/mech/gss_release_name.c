
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
 * Free a name
 *
 * import_name can point to NULL or be NULL, or a pointer to a
 * gss_name_t structure. If it was a pointer to gss_name_t, the
 * pointer will be set to NULL on success and failure.
 *
 * @param minor_status minor status code
 * @param input_name name to free
 *
 * @returns a gss_error code, see gss_display_status() about printing
 *        the error code.
 *
 * @ingroup gssapi
 */
GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_release_name(OM_uint32 *minor_status,
    gss_name_t *input_name)
{
	struct _gss_name *name;

	*minor_status = 0;

	if (input_name == NULL || *input_name == NULL)
	    return GSS_S_COMPLETE;

	name = (struct _gss_name *) *input_name;

	if (name->gn_type.elements)
		free(name->gn_type.elements);
	while (HEIM_SLIST_FIRST(&name->gn_mn)) {
		struct _gss_mechanism_name *mn;
		mn = HEIM_SLIST_FIRST(&name->gn_mn);
		HEIM_SLIST_REMOVE_HEAD(&name->gn_mn, gmn_link);
		mn->gmn_mech->gm_release_name(minor_status,
					      &mn->gmn_name);
		free(mn);
	}
	gss_release_buffer(minor_status, &name->gn_value);
	free(name);
	*input_name = GSS_C_NO_NAME;

	return (GSS_S_COMPLETE);
}