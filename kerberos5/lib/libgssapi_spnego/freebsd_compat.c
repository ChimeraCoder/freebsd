
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
#include <mech_switch.h>

gss_OID_desc __gss_c_nt_hostbased_service_oid_desc =
    {10, (void *)("\x2a\x86\x48\x86\xf7\x12" "\x01\x02\x01\x04")};

const char *
_gss_name_prefix(void)
{
	return "_gss_spnego";
}

void
gss_mg_collect_error(gss_OID mech, OM_uint32 maj, OM_uint32 min)
{
	_gss_mg_collect_error(mech, maj, min);
}

OM_uint32 _gss_spnego_display_status
           (OM_uint32 * minor_status,
            OM_uint32 status_value,
            int status_type,
            const gss_OID mech_type,
            OM_uint32 * message_context,
            gss_buffer_t status_string
           )
{
    return GSS_S_FAILURE;
}

OM_uint32 _gss_spnego_add_cred (
            OM_uint32 * minor_status,
            const gss_cred_id_t input_cred_handle,
            const gss_name_t desired_name,
            const gss_OID desired_mech,
            gss_cred_usage_t cred_usage,
            OM_uint32 initiator_time_req,
            OM_uint32 acceptor_time_req,
            gss_cred_id_t * output_cred_handle,
            gss_OID_set * actual_mechs,
            OM_uint32 * initiator_time_rec,
            OM_uint32 * acceptor_time_rec
           )
{
	return gss_add_cred(minor_status,
                       input_cred_handle,
                       desired_name,
                       desired_mech,
                       cred_usage,
                       initiator_time_req,
                       acceptor_time_req,
                       output_cred_handle,
                       actual_mechs,
                       initiator_time_rec,
                       acceptor_time_rec);
}