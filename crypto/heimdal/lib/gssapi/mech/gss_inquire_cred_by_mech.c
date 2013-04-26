
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
gss_inquire_cred_by_mech(OM_uint32 *minor_status,
    const gss_cred_id_t cred_handle,
    const gss_OID mech_type,
    gss_name_t *cred_name,
    OM_uint32 *initiator_lifetime,
    OM_uint32 *acceptor_lifetime,
    gss_cred_usage_t *cred_usage)
{
	OM_uint32 major_status;
	gssapi_mech_interface m;
	struct _gss_mechanism_cred *mcp;
	gss_cred_id_t mc;
	gss_name_t mn;
	struct _gss_name *name;

	*minor_status = 0;
	if (cred_name)
	    *cred_name = GSS_C_NO_NAME;
	if (initiator_lifetime)
	    *initiator_lifetime = 0;
	if (acceptor_lifetime)
	    *acceptor_lifetime = 0;
	if (cred_usage)
	    *cred_usage = 0;

	m = __gss_get_mechanism(mech_type);
	if (!m)
		return (GSS_S_NO_CRED);

	if (cred_handle != GSS_C_NO_CREDENTIAL) {
		struct _gss_cred *cred = (struct _gss_cred *) cred_handle;
		HEIM_SLIST_FOREACH(mcp, &cred->gc_mc, gmc_link)
			if (mcp->gmc_mech == m)
				break;
		if (!mcp)
			return (GSS_S_NO_CRED);
		mc = mcp->gmc_cred;
	} else {
		mc = GSS_C_NO_CREDENTIAL;
	}

	major_status = m->gm_inquire_cred_by_mech(minor_status, mc, mech_type,
	    &mn, initiator_lifetime, acceptor_lifetime, cred_usage);
	if (major_status != GSS_S_COMPLETE) {
		_gss_mg_error(m, major_status, *minor_status);
		return (major_status);
	}

	if (cred_name) {
	    name = _gss_make_name(m, mn);
	    if (!name) {
		m->gm_release_name(minor_status, &mn);
		return (GSS_S_NO_CRED);
	    }
	    *cred_name = (gss_name_t) name;
	} else
	    m->gm_release_name(minor_status, &mn);


	return (GSS_S_COMPLETE);
}