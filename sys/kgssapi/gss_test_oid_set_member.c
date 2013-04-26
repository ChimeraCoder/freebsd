
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/kobj.h>
#include <sys/malloc.h>

#include <kgssapi/gssapi.h>
#include <kgssapi/gssapi_impl.h>

OM_uint32
gss_test_oid_set_member(OM_uint32 *minor_status,
    const gss_OID member,
    const gss_OID_set set,
    int *present)
{
	size_t i;

	*present = 0;
	for (i = 0; i < set->count; i++)
		if (kgss_oid_equal(member, &set->elements[i]))
			*present = 1;

	*minor_status = 0;
	return (GSS_S_COMPLETE);
}