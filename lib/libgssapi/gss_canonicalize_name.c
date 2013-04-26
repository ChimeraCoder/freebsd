
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
#include <string.h>
#include <errno.h>

#include "mech_switch.h"
#include "name.h"

OM_uint32
gss_canonicalize_name(OM_uint32 *minor_status,
    const gss_name_t input_name,
    const gss_OID mech_type,
    gss_name_t *output_name)
{
	OM_uint32 major_status;
	struct _gss_name *name = (struct _gss_name *) input_name;
	struct _gss_mechanism_name *mn;
	struct _gss_mech_switch *m = _gss_find_mech_switch(mech_type);
	gss_name_t new_canonical_name;

	*minor_status = 0;
	*output_name = 0;

	major_status = _gss_find_mn(minor_status, name, mech_type, &mn);
	if (major_status)
		return (major_status);

	m = mn->gmn_mech;
	major_status = m->gm_canonicalize_name(minor_status,
	    mn->gmn_name, mech_type, &new_canonical_name);
	if (major_status) {
		_gss_mg_error(m, major_status, *minor_status);
		return (major_status);
	}

	/*
	 * Now we make a new name and mark it as an MN.
	 */
	*minor_status = 0;
	name = malloc(sizeof(struct _gss_name));
	if (!name) {
		m->gm_release_name(minor_status, &new_canonical_name);
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}
	memset(name, 0, sizeof(struct _gss_name));

	mn = malloc(sizeof(struct _gss_mechanism_name));
	if (!mn) {
		m->gm_release_name(minor_status, &new_canonical_name);
		free(name);
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}

	SLIST_INIT(&name->gn_mn);
	mn->gmn_mech = m;
	mn->gmn_mech_oid = &m->gm_mech_oid;
	mn->gmn_name = new_canonical_name;
	SLIST_INSERT_HEAD(&name->gn_mn, mn, gmn_link);

	*output_name = (gss_name_t) name;

	return (GSS_S_COMPLETE);
}