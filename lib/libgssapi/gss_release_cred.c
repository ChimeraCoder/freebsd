
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
gss_release_cred(OM_uint32 *minor_status, gss_cred_id_t *cred_handle)
{
	struct _gss_cred *cred = (struct _gss_cred *) *cred_handle;
	struct _gss_mechanism_cred *mc;

	if (*cred_handle == GSS_C_NO_CREDENTIAL)
	    return (GSS_S_COMPLETE);

	while (SLIST_FIRST(&cred->gc_mc)) {
		mc = SLIST_FIRST(&cred->gc_mc);
		SLIST_REMOVE_HEAD(&cred->gc_mc, gmc_link);
		mc->gmc_mech->gm_release_cred(minor_status, &mc->gmc_cred);
		free(mc);
	}
	free(cred);

	*minor_status = 0;
	*cred_handle = GSS_C_NO_CREDENTIAL;
	return (GSS_S_COMPLETE);
}