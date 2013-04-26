
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
#include <sys/types.h>

#include <stand.h>

#include "libofw.h"
#include "openfirm.h"

static void		*heap_base = 0;
static unsigned int	heap_size = 0;

struct ofw_mapping {
        vm_offset_t     va;
        int             len;
        vm_offset_t     pa;
        int             mode;
};

struct ofw_mapping2 {
        vm_offset_t     va;
        int             len;
        vm_offset_t     pa_hi;
        vm_offset_t     pa_lo;
        int             mode;
};

void
ofw_memmap(int acells)
{
	struct		ofw_mapping *mapptr;
	struct		ofw_mapping2 *mapptr2;
        phandle_t	mmup;
        int		nmapping, i;
        u_char		mappings[256 * sizeof(struct ofw_mapping2)];
        char		lbuf[80];

	mmup = OF_instance_to_package(mmu);

	bzero(mappings, sizeof(mappings));

	nmapping = OF_getprop(mmup, "translations", mappings, sizeof(mappings));
	if (nmapping == -1) {
		printf("Could not get memory map (%d)\n",
		    nmapping);
		return;
	}

	pager_open();
	if (acells == 1) {
		nmapping /= sizeof(struct ofw_mapping);
		mapptr = (struct ofw_mapping *) mappings;

		printf("%17s\t%17s\t%8s\t%6s\n", "Virtual Range",
		    "Physical Range", "#Pages", "Mode");

		for (i = 0; i < nmapping; i++) {
			sprintf(lbuf, "%08x-%08x\t%08x-%08x\t%8d\t%6x\n",
				mapptr[i].va,
				mapptr[i].va + mapptr[i].len,
				mapptr[i].pa,
				mapptr[i].pa + mapptr[i].len,
				mapptr[i].len / 0x1000,
				mapptr[i].mode);
			if (pager_output(lbuf))
				break;
		}
	} else {
		nmapping /= sizeof(struct ofw_mapping2);
		mapptr2 = (struct ofw_mapping2 *) mappings;

		printf("%17s\t%17s\t%8s\t%6s\n", "Virtual Range",
		       "Physical Range", "#Pages", "Mode");

		for (i = 0; i < nmapping; i++) {
			sprintf(lbuf, "%08x-%08x\t%08x-%08x\t%8d\t%6x\n",
				mapptr2[i].va,
				mapptr2[i].va + mapptr2[i].len,
				mapptr2[i].pa_lo,
				mapptr2[i].pa_lo + mapptr2[i].len,
				mapptr2[i].len / 0x1000,
				mapptr2[i].mode);
			if (pager_output(lbuf))
				break;
		}
	}
	pager_close();
}

void *
ofw_alloc_heap(unsigned int size)
{
	phandle_t	memoryp, root;
	cell_t		available[4];
	cell_t		acells;

	root = OF_finddevice("/");
	acells = 1;
	OF_getprop(root, "#address-cells", &acells, sizeof(acells));

	memoryp = OF_instance_to_package(memory);
	OF_getprop(memoryp, "available", available, sizeof(available));

	heap_base = OF_claim((void *)available[acells-1], size,
	    sizeof(register_t));

	if (heap_base != (void *)-1) {
		heap_size = size;
	}

	return (heap_base);
}

void
ofw_release_heap(void)
{
	OF_release(heap_base, heap_size);
}