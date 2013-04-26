
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
#include <stdlib.h>
#include <errno.h>

#include "mech_switch.h"
#include "cred.h"

OM_uint32
gss_set_cred_option (OM_uint32 *minor_status,
		     gss_cred_id_t *cred_handle,
		     const gss_OID object,
		     const gss_buffer_t value)
{
	struct _gss_cred *cred = (struct _gss_cred *) *cred_handle;
	OM_uint32	major_status = GSS_S_COMPLETE;
	struct _gss_mechanism_cred *mc;
	int one_ok = 0;

	*minor_status = 0;

	_gss_load_mech();

	if (cred == NULL) {
		struct _gss_mech_switch *m;

		cred = malloc(sizeof(*cred));
		if (cred == NULL)
		    return GSS_S_FAILURE;

		SLIST_INIT(&cred->gc_mc);

		SLIST_FOREACH(m, &_gss_mechs, gm_link) {

			if (m->gm_set_cred_option == NULL)
				continue;

			mc = malloc(sizeof(*mc));
			if (mc == NULL) {
			    *cred_handle = (gss_cred_id_t)cred;
			    gss_release_cred(minor_status, cred_handle);
			    *minor_status = ENOMEM;
			    return GSS_S_FAILURE;
			}

			mc->gmc_mech = m;
			mc->gmc_mech_oid = &m->gm_mech_oid;
			mc->gmc_cred = GSS_C_NO_CREDENTIAL;

			major_status = m->gm_set_cred_option(
			    minor_status, &mc->gmc_cred, object, value);

			if (major_status) {
				free(mc);
				continue;
			}
			one_ok = 1;
			SLIST_INSERT_HEAD(&cred->gc_mc, mc, gmc_link);
		}
		*cred_handle = (gss_cred_id_t)cred;
		if (!one_ok) {
			OM_uint32 junk;
			gss_release_cred(&junk, cred_handle);
		}
	} else {
		struct _gss_mech_switch *m;

		SLIST_FOREACH(mc, &cred->gc_mc, gmc_link) {
			m = mc->gmc_mech;
	
			if (m == NULL)
				return GSS_S_BAD_MECH;
	
			if (m->gm_set_cred_option == NULL)
				continue;
	
			major_status = m->gm_set_cred_option(minor_status,
			    &mc->gmc_cred, object, value);
			if (major_status == GSS_S_COMPLETE)
				one_ok = 1;
			else
				_gss_mg_error(m, major_status, *minor_status);

		}
	}
	if (one_ok) {
		*minor_status = 0;
		return (GSS_S_COMPLETE);
	}
	return (major_status);
}