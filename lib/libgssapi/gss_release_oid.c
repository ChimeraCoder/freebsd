
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

/* RCSID("$Id: gss_release_oid.c 17747 2006-06-30 09:34:54Z lha $"); */

OM_uint32
gss_release_oid(OM_uint32 *minor_status, gss_OID *oid)
{
	gss_OID o = *oid;

	*oid = GSS_C_NO_OID;

	if (minor_status != NULL)
		*minor_status = 0;

	if (o == GSS_C_NO_OID)
		return (GSS_S_COMPLETE);

	if (o->elements != NULL) {
		free(o->elements);
		o->elements = NULL;
	}
	o->length = 0;
	free(o);

	return (GSS_S_COMPLETE);
}