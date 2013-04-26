
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
gss_store_cred(OM_uint32         *minor_status,
	       gss_cred_id_t     input_cred_handle,
	       gss_cred_usage_t  cred_usage,
	       const gss_OID     desired_mech,
	       OM_uint32         overwrite_cred,
	       OM_uint32         default_cred,
	       gss_OID_set       *elements_stored,
	       gss_cred_usage_t  *cred_usage_stored)
{
    struct _gss_cred *cred = (struct _gss_cred *) input_cred_handle;
    struct _gss_mechanism_cred *mc;
    OM_uint32 maj, junk;

    if (minor_status == NULL)
	return GSS_S_FAILURE;
    if (elements_stored)
	*elements_stored = NULL;
    if (cred_usage_stored)
	*cred_usage_stored = 0;

    if (cred == NULL)
	return GSS_S_NO_CONTEXT;

    if (elements_stored) {
	maj = gss_create_empty_oid_set(minor_status, elements_stored);
	if (maj != GSS_S_COMPLETE)
	    return maj;
    }

    HEIM_SLIST_FOREACH(mc, &cred->gc_mc, gmc_link) {
	gssapi_mech_interface m = mc->gmc_mech;

	if (m == NULL || m->gm_store_cred == NULL)
	    continue;

	if (desired_mech) {
	    maj = gss_oid_equal(&m->gm_mech_oid, desired_mech);
	    if (maj != 0)
		continue;
	}

	maj = (m->gm_store_cred)(minor_status, mc->gmc_cred,
				 cred_usage, desired_mech, overwrite_cred,
				 default_cred, NULL, cred_usage_stored);
	if (maj != GSS_S_COMPLETE) {
	    gss_release_oid_set(&junk, elements_stored);
	    return maj;
	}

	if (elements_stored) {
	    gss_add_oid_set_member(&junk,
				   &m->gm_mech_oid,
				   elements_stored);
	}

    }
    return GSS_S_COMPLETE;
}