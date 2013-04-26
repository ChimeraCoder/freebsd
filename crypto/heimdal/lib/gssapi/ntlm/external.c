
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

#include "ntlm.h"

#if 0
static gss_mo_desc ntlm_mo[] = {
    {
	GSS_C_MA_SASL_MECH_NAME,
	GSS_MO_MA,
	"SASL mech name",
	rk_UNCONST("NTLM"),
	_gss_mo_get_ctx_as_string,
	NULL
    },
    {
	GSS_C_MA_MECH_NAME,
	GSS_MO_MA,
	"Mechanism name",
	rk_UNCONST("NTLMSPP"),
	_gss_mo_get_ctx_as_string,
	NULL
    },
    {
	GSS_C_MA_MECH_DESCRIPTION,
	GSS_MO_MA,
	"Mechanism description",
	rk_UNCONST("Heimdal NTLMSSP Mechanism"),
	_gss_mo_get_ctx_as_string,
	NULL
    }
};

#endif

static gssapi_mech_interface_desc ntlm_mech = {
    GMI_VERSION,
    "ntlm",
    {10, rk_UNCONST("\x2b\x06\x01\x04\x01\x82\x37\x02\x02\x0a") },
    0,
    _gss_ntlm_acquire_cred,
    _gss_ntlm_release_cred,
    _gss_ntlm_init_sec_context,
    _gss_ntlm_accept_sec_context,
    _gss_ntlm_process_context_token,
    _gss_ntlm_delete_sec_context,
    _gss_ntlm_context_time,
    _gss_ntlm_get_mic,
    _gss_ntlm_verify_mic,
    _gss_ntlm_wrap,
    _gss_ntlm_unwrap,
    _gss_ntlm_display_status,
    NULL,
    _gss_ntlm_compare_name,
    _gss_ntlm_display_name,
    _gss_ntlm_import_name,
    _gss_ntlm_export_name,
    _gss_ntlm_release_name,
    _gss_ntlm_inquire_cred,
    _gss_ntlm_inquire_context,
    _gss_ntlm_wrap_size_limit,
    _gss_ntlm_add_cred,
    _gss_ntlm_inquire_cred_by_mech,
    _gss_ntlm_export_sec_context,
    _gss_ntlm_import_sec_context,
    _gss_ntlm_inquire_names_for_mech,
    _gss_ntlm_inquire_mechs_for_name,
    _gss_ntlm_canonicalize_name,
    _gss_ntlm_duplicate_name,
    _gss_ntlm_inquire_sec_context_by_oid,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    _gss_ntlm_iter_creds_f,
    _gss_ntlm_destroy_cred,
    NULL,
    NULL,
    NULL,
    NULL,
#if 0
    ntlm_mo,
    sizeof(ntlm_mo) / sizeof(ntlm_mo[0]),
#else
    NULL,
    0,
#endif
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

gssapi_mech_interface
__gss_ntlm_initialize(void)
{
	return &ntlm_mech;
}