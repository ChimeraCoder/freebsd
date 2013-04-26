
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
#include <sys/bus.h>
#include <sys/malloc.h>
#include <machine/bus.h>
#include <sys/rman.h>

#include <machine/intr_machdep.h>
#include <machine/resource.h>

#include <isa/isareg.h>
#include <isa/isavar.h>
#include <isa/isa_common.h>

void
isa_init(device_t dev)
{
}

struct resource *
isa_alloc_resource(device_t bus, device_t child, int type, int *rid,
    u_long start, u_long end, u_long count, u_int flags)
{
	struct isa_device* idev = DEVTOISA(child);
	struct resource_list *rl = &idev->id_resources;
	int isdefault, passthrough, rids;

	isdefault = (start == 0UL && end == ~0UL) ? 1 : 0;
	passthrough = (device_get_parent(child) != bus) ? 1 : 0;

	if (!passthrough && !isdefault &&
	    resource_list_find(rl, type, *rid) == NULL) {
		switch (type) {
		case SYS_RES_IOPORT:	rids = ISA_PNP_NPORT; break;
		case SYS_RES_IRQ:	rids = ISA_PNP_NIRQ; break;
		case SYS_RES_MEMORY:	rids = ISA_PNP_NMEM; break;
		default:		rids = 0; break;
		}
		if (*rid < 0 || *rid >= rids)
			return (NULL);

		resource_list_add(rl, type, *rid, start, end, count);
	}

	return (resource_list_alloc(rl, bus, child, type, rid, start, end,
	    count, flags));
}

int
isa_release_resource(device_t bus, device_t child, int type, int rid,
    struct resource *r)
{
	struct isa_device* idev = DEVTOISA(child);
	struct resource_list *rl = &idev->id_resources;

	return (resource_list_release(rl, bus, child, type, rid, r));
}