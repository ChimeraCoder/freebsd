
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
_gss_acquire_mech_cred(OM_uint32 *minor_status,
		       gssapi_mech_interface m,
		       const struct _gss_mechanism_name *mn,
		       gss_const_OID credential_type,
		       const void *credential_data,
		       OM_uint32 time_req,
		       gss_const_OID desired_mech,
		       gss_cred_usage_t cred_usage,
		       struct _gss_mechanism_cred **output_cred_handle)
{
    OM_uint32 major_status;
    struct _gss_mechanism_cred *mc;
    gss_OID_set_desc set2;

    *output_cred_handle = NULL;

    mc = calloc(1, sizeof(struct _gss_mechanism_cred));
    if (mc == NULL) {
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }

    mc->gmc_mech = m;
    mc->gmc_mech_oid = &m->gm_mech_oid;

    set2.count = 1;
    set2.elements = mc->gmc_mech_oid;

    if (m->gm_acquire_cred_ext) {
	major_status = m->gm_acquire_cred_ext(minor_status,
					      mn->gmn_name,
					      credential_type,
					      credential_data,
					      time_req,
					      mc->gmc_mech_oid,
					      cred_usage,
					      &mc->gmc_cred);
    } else if (gss_oid_equal(credential_type, GSS_C_CRED_PASSWORD) &&
		m->gm_compat &&
		m->gm_compat->gmc_acquire_cred_with_password) {
	/*
	 * Shim for mechanisms that adhere to API-as-SPI and do not
	 * implement gss_acquire_cred_ext().
	 */

	major_status = m->gm_compat->gmc_acquire_cred_with_password(minor_status,
				mn->gmn_name,
				(const gss_buffer_t)credential_data,
				time_req,
				&set2,
				cred_usage,
				&mc->gmc_cred,
				NULL,
				NULL);
    } else if (credential_type == GSS_C_NO_OID) {
	major_status = m->gm_acquire_cred(minor_status,
					  mn->gmn_name,
					  time_req,
					  &set2,
					  cred_usage,
					  &mc->gmc_cred,
					  NULL,
					  NULL);
    } else {
	major_status = GSS_S_UNAVAILABLE;
	free(mc);
	mc= NULL;
    }

    *output_cred_handle = mc;
    return major_status;
}

OM_uint32
_gss_acquire_cred_ext(OM_uint32 *minor_status,
    const gss_name_t desired_name,
    gss_const_OID credential_type,
    const void *credential_data,
    OM_uint32 time_req,
    gss_const_OID desired_mech,
    gss_cred_usage_t cred_usage,
    gss_cred_id_t *output_cred_handle)
{
    OM_uint32 major_status;
    struct _gss_name *name = (struct _gss_name *) desired_name;
    gssapi_mech_interface m;
    struct _gss_cred *cred;
    gss_OID_set_desc set, *mechs;
    size_t i;

    *minor_status = 0;
    if (output_cred_handle == NULL)
	return GSS_S_CALL_INACCESSIBLE_READ;

    _gss_load_mech();

    if (desired_mech != GSS_C_NO_OID) {
	int match = 0;

	gss_test_oid_set_member(minor_status, (gss_OID)desired_mech,
				_gss_mech_oids, &match);
	if (!match)
	    return GSS_S_BAD_MECH;

	set.count = 1;
	set.elements = (gss_OID)desired_mech;
	mechs = &set;
    } else
	mechs = _gss_mech_oids;

    cred = calloc(1, sizeof(*cred));
    if (cred == NULL) {
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }

    HEIM_SLIST_INIT(&cred->gc_mc);

    for (i = 0; i < mechs->count; i++) {
	struct _gss_mechanism_name *mn = NULL;
	struct _gss_mechanism_cred *mc = NULL;
	gss_name_t desired_mech_name = GSS_C_NO_NAME;

	m = __gss_get_mechanism(&mechs->elements[i]);
	if (!m)
	    continue;

	if (desired_name != GSS_C_NO_NAME) {
	    major_status = _gss_find_mn(minor_status, name,
					&mechs->elements[i], &mn);
	    if (major_status != GSS_S_COMPLETE)
		continue;

	    desired_mech_name = mn->gmn_name;
	}

	major_status = _gss_acquire_mech_cred(minor_status, m, mn,
					      credential_type, credential_data,
					      time_req, desired_mech, cred_usage,
					      &mc);
	if (GSS_ERROR(major_status))
	    continue;

	HEIM_SLIST_INSERT_HEAD(&cred->gc_mc, mc, gmc_link);
    }

    /*
     * If we didn't manage to create a single credential, return
     * an error.
     */
    if (!HEIM_SLIST_FIRST(&cred->gc_mc)) {
	free(cred);
	*minor_status = 0;
	return GSS_S_NO_CRED;
    }

    *output_cred_handle = (gss_cred_id_t) cred;
    *minor_status = 0;
    return GSS_S_COMPLETE;
}