
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
gss_inquire_cred_by_oid (OM_uint32 *minor_status,
			 const gss_cred_id_t cred_handle,
			 const gss_OID desired_object,
			 gss_buffer_set_t *data_set)
{
	struct _gss_cred *cred = (struct _gss_cred *) cred_handle;
	OM_uint32		status = GSS_S_COMPLETE;
	struct _gss_mechanism_cred *mc;
	gssapi_mech_interface	m;
	gss_buffer_set_t set = GSS_C_NO_BUFFER_SET;

	*minor_status = 0;
	*data_set = GSS_C_NO_BUFFER_SET;

	if (cred == NULL)
		return GSS_S_NO_CRED;

	HEIM_SLIST_FOREACH(mc, &cred->gc_mc, gmc_link) {
		gss_buffer_set_t rset = GSS_C_NO_BUFFER_SET;
		size_t i;

		m = mc->gmc_mech;
		if (m == NULL) {
	       		gss_release_buffer_set(minor_status, &set);
			*minor_status = 0;
			return GSS_S_BAD_MECH;
		}

		if (m->gm_inquire_cred_by_oid == NULL)
			continue;

		status = m->gm_inquire_cred_by_oid(minor_status,
		    mc->gmc_cred, desired_object, &rset);
		if (status != GSS_S_COMPLETE)
			continue;

		for (i = 0; i < rset->count; i++) {
			status = gss_add_buffer_set_member(minor_status,
			     &rset->elements[i], &set);
			if (status != GSS_S_COMPLETE)
				break;
		}
		gss_release_buffer_set(minor_status, &rset);
	}
	if (set == GSS_C_NO_BUFFER_SET)
		status = GSS_S_FAILURE;
	*data_set = set;
	*minor_status = 0;
	return status;
}