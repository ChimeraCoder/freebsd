
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

#include "gsskrb5_locl.h"

OM_uint32 GSSAPI_CALLCONV _gsskrb5_inquire_cred_by_mech (
    OM_uint32 * minor_status,
	const gss_cred_id_t cred_handle,
	const gss_OID mech_type,
	gss_name_t * name,
	OM_uint32 * initiator_lifetime,
	OM_uint32 * acceptor_lifetime,
	gss_cred_usage_t * cred_usage
    )
{
    gss_cred_usage_t usage;
    OM_uint32 maj_stat;
    OM_uint32 lifetime;

    maj_stat =
	_gsskrb5_inquire_cred (minor_status, cred_handle,
			       name, &lifetime, &usage, NULL);
    if (maj_stat)
	return maj_stat;

    if (initiator_lifetime) {
	if (usage == GSS_C_INITIATE || usage == GSS_C_BOTH)
	    *initiator_lifetime = lifetime;
	else
	    *initiator_lifetime = 0;
    }

    if (acceptor_lifetime) {
	if (usage == GSS_C_ACCEPT || usage == GSS_C_BOTH)
	    *acceptor_lifetime = lifetime;
	else
	    *acceptor_lifetime = 0;
    }

    if (cred_usage)
	*cred_usage = usage;

    return GSS_S_COMPLETE;
}