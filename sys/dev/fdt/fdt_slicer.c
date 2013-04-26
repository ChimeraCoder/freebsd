
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
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/slicer.h>

#include <dev/fdt/fdt_common.h>

#define DEBUG
#undef DEBUG

#ifdef DEBUG
#define debugf(fmt, args...) do { printf("%s(): ", __func__);	\
    printf(fmt,##args); } while (0)
#else
#define debugf(fmt, args...)
#endif

int
flash_fill_slices(device_t dev, struct flash_slice *slices, int *slices_num)
{
	char *slice_name;
	phandle_t dt_node, dt_child;
	u_long base, size;
	int i;
	ssize_t name_len;

	/*
	 * We assume the caller provides buffer for FLASH_SLICES_MAX_NUM
	 * flash_slice structures.
	 */
	if (slices == NULL) {
		*slices_num = 0;
		return (ENOMEM);
	}

	dt_node = ofw_bus_get_node(dev);
	for (dt_child = OF_child(dt_node), i = 0; dt_child != 0;
	    dt_child = OF_peer(dt_child)) {

		if (i == FLASH_SLICES_MAX_NUM) {
			debugf("not enough buffer for slice i=%d\n", i);
			break;
		}

		/*
		 * Retrieve start and size of the slice.
		 */
		if (fdt_regsize(dt_child, &base, &size) != 0) {
			debugf("error during processing reg property, i=%d\n",
			    i);
			continue;
		}

		if (size == 0) {
			debugf("slice i=%d with no size\n", i);
			continue;
		}

		/*
		 * Retrieve label.
		 */
		name_len = OF_getprop_alloc(dt_child, "label", sizeof(char),
		    (void **)&slice_name);
		if (name_len <= 0) {
			/* Use node name if no label defined */
			name_len = OF_getprop_alloc(dt_child, "name", sizeof(char),
			    (void **)&slice_name);
			if (name_len <= 0) {
				debugf("slice i=%d with no name\n", i);
				slice_name = NULL;
			}
		}

		/*
		 * Fill slice entry data.
		 */
		slices[i].base = base;
		slices[i].size = size;
		slices[i].label = slice_name;
		i++;
	}

	*slices_num = i;
	return (0);
}