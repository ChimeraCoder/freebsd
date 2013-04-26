
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
#include "name.h"

OM_uint32
gss_release_name(OM_uint32 *minor_status,
    gss_name_t *input_name)
{
	struct _gss_name *name = (struct _gss_name *) *input_name;

	*minor_status = 0;
	if (name) {
		if (name->gn_type.elements)
			free(name->gn_type.elements);
		while (SLIST_FIRST(&name->gn_mn)) {
			struct _gss_mechanism_name *mn;
			mn = SLIST_FIRST(&name->gn_mn);
			SLIST_REMOVE_HEAD(&name->gn_mn, gmn_link);
			mn->gmn_mech->gm_release_name(minor_status,
			    &mn->gmn_name);
			free(mn);
		}
		gss_release_buffer(minor_status, &name->gn_value);
		free(name);
		*input_name = GSS_C_NO_NAME;
	}
	return (GSS_S_COMPLETE);
}