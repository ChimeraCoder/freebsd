
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

struct _gss_mechanism_cred *
_gss_copy_cred(struct _gss_mechanism_cred *mc)
{
	struct _gss_mechanism_cred *new_mc;
	gssapi_mech_interface m = mc->gmc_mech;
	OM_uint32 major_status, minor_status;
	gss_name_t name;
	gss_cred_id_t cred;
	OM_uint32 initiator_lifetime, acceptor_lifetime;
	gss_cred_usage_t cred_usage;

	major_status = m->gm_inquire_cred_by_mech(&minor_status,
	    mc->gmc_cred, mc->gmc_mech_oid,
	    &name, &initiator_lifetime, &acceptor_lifetime, &cred_usage);
	if (major_status) {
		_gss_mg_error(m, major_status, minor_status);
		return (0);
	}

	major_status = m->gm_add_cred(&minor_status,
	    GSS_C_NO_CREDENTIAL, name, mc->gmc_mech_oid,
	    cred_usage, initiator_lifetime, acceptor_lifetime,
	    &cred, 0, 0, 0);
	m->gm_release_name(&minor_status, &name);

	if (major_status) {
		_gss_mg_error(m, major_status, minor_status);
		return (0);
	}

	new_mc = malloc(sizeof(struct _gss_mechanism_cred));
	if (!new_mc) {
		m->gm_release_cred(&minor_status, &cred);
		return (0);
	}
	new_mc->gmc_mech = m;
	new_mc->gmc_mech_oid = &m->gm_mech_oid;
	new_mc->gmc_cred = cred;

	return (new_mc);
}

GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_add_cred(OM_uint32 *minor_status,
    const gss_cred_id_t input_cred_handle,
    const gss_name_t desired_name,
    const gss_OID desired_mech,
    gss_cred_usage_t cred_usage,
    OM_uint32 initiator_time_req,
    OM_uint32 acceptor_time_req,
    gss_cred_id_t *output_cred_handle,
    gss_OID_set *actual_mechs,
    OM_uint32 *initiator_time_rec,
    OM_uint32 *acceptor_time_rec)
{
	OM_uint32 major_status;
	gssapi_mech_interface m;
	struct _gss_cred *cred = (struct _gss_cred *) input_cred_handle;
	struct _gss_cred *new_cred;
	gss_cred_id_t release_cred;
	struct _gss_mechanism_cred *mc, *target_mc, *copy_mc;
	struct _gss_mechanism_name *mn;
	OM_uint32 junk;

	*minor_status = 0;
	*output_cred_handle = GSS_C_NO_CREDENTIAL;
	if (initiator_time_rec)
	    *initiator_time_rec = 0;
	if (acceptor_time_rec)
	    *acceptor_time_rec = 0;
	if (actual_mechs)
	    *actual_mechs = GSS_C_NO_OID_SET;

	new_cred = malloc(sizeof(struct _gss_cred));
	if (!new_cred) {
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}
	HEIM_SLIST_INIT(&new_cred->gc_mc);

	/*
	 * We go through all the mc attached to the input_cred_handle
	 * and check the mechanism. If it matches, we call
	 * gss_add_cred for that mechanism, otherwise we copy the mc
	 * to new_cred.
	 */
	target_mc = 0;
	if (cred) {
		HEIM_SLIST_FOREACH(mc, &cred->gc_mc, gmc_link) {
			if (gss_oid_equal(mc->gmc_mech_oid, desired_mech)) {
				target_mc = mc;
			}
			copy_mc = _gss_copy_cred(mc);
			if (!copy_mc) {
				release_cred = (gss_cred_id_t)new_cred;
				gss_release_cred(&junk, &release_cred);
				*minor_status = ENOMEM;
				return (GSS_S_FAILURE);
			}
			HEIM_SLIST_INSERT_HEAD(&new_cred->gc_mc, copy_mc, gmc_link);
		}
	}

	/*
	 * Figure out a suitable mn, if any.
	 */
	if (desired_name) {
		major_status = _gss_find_mn(minor_status,
					    (struct _gss_name *) desired_name,
					    desired_mech,
					    &mn);
		if (major_status != GSS_S_COMPLETE) {
			free(new_cred);
			return major_status;
		}
	} else {
		mn = 0;
	}

	m = __gss_get_mechanism(desired_mech);

	mc = malloc(sizeof(struct _gss_mechanism_cred));
	if (!mc) {
		release_cred = (gss_cred_id_t)new_cred;
		gss_release_cred(&junk, &release_cred);
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}
	mc->gmc_mech = m;
	mc->gmc_mech_oid = &m->gm_mech_oid;

	major_status = m->gm_add_cred(minor_status,
	    target_mc ? target_mc->gmc_cred : GSS_C_NO_CREDENTIAL,
	    desired_name ? mn->gmn_name : GSS_C_NO_NAME,
	    desired_mech,
	    cred_usage,
	    initiator_time_req,
	    acceptor_time_req,
	    &mc->gmc_cred,
	    actual_mechs,
	    initiator_time_rec,
	    acceptor_time_rec);

	if (major_status) {
		_gss_mg_error(m, major_status, *minor_status);
		release_cred = (gss_cred_id_t)new_cred;
		gss_release_cred(&junk, &release_cred);
		free(mc);
		return (major_status);
	}
	HEIM_SLIST_INSERT_HEAD(&new_cred->gc_mc, mc, gmc_link);
	*output_cred_handle = (gss_cred_id_t) new_cred;

	return (GSS_S_COMPLETE);
}