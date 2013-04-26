
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

OM_uint32 GSSAPI_CALLCONV _gss_ntlm_release_cred
           (OM_uint32 * minor_status,
            gss_cred_id_t * cred_handle
           )
{
    ntlm_cred cred;

    if (minor_status)
	*minor_status = 0;

    if (cred_handle == NULL || *cred_handle == GSS_C_NO_CREDENTIAL)
	return GSS_S_COMPLETE;

    cred = (ntlm_cred)*cred_handle;
    *cred_handle = GSS_C_NO_CREDENTIAL;

    if (cred->username)
	free(cred->username);
    if (cred->domain)
	free(cred->domain);
    if (cred->key.data) {
	memset(cred->key.data, 0, cred->key.length);
	free(cred->key.data);
    }

    return GSS_S_COMPLETE;
}