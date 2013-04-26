
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

#include <unistd.h>
#include <gssapi/gssapi.h>

#include "mech_switch.h"
#include "name.h"
#include "utils.h"

OM_uint32
gss_pname_to_uid(OM_uint32 *minor_status, const gss_name_t pname,
    const gss_OID mech, uid_t *uidp)
{
	struct _gss_name *name = (struct _gss_name *) pname;
	struct _gss_mech_switch *m;
	struct _gss_mechanism_name *mn;
	OM_uint32 major_status;

	*minor_status = 0;

	if (pname == GSS_C_NO_NAME)
		return (GSS_S_BAD_NAME);

	m = _gss_find_mech_switch(mech);
	if (!m)
		return (GSS_S_BAD_MECH);

	if (m->gm_pname_to_uid == NULL)
		return (GSS_S_UNAVAILABLE);
    
	major_status = _gss_find_mn(minor_status, name, mech, &mn);
	if (major_status != GSS_S_COMPLETE) {
		_gss_mg_error(m, major_status, *minor_status);
		return (major_status);
	}

	major_status = (*m->gm_pname_to_uid)(minor_status, mn->gmn_name,
	    mech, uidp);
	if (major_status != GSS_S_COMPLETE)
		_gss_mg_error(m, major_status, *minor_status);

	return (major_status);
}