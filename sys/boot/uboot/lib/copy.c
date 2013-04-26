
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

#include <stand.h>
#include <stdint.h>

#include "api_public.h"
#include "glue.h"

/*
 * MD primitives supporting placement of module data 
 */

void *
uboot_vm_translate(vm_offset_t o) {
	struct sys_info *si;
	static uintptr_t start = 0;
	static size_t size = 0;
	int i;

	if (size == 0) {
		if ((si = ub_get_sys_info()) == NULL)
			panic("could not retrieve system info");

		/* Find start/size of largest DRAM block. */
		for (i = 0; i < si->mr_no; i++) {
			if (si->mr[i].flags == MR_ATTR_DRAM
			    && si->mr[i].size > size) {
				start = si->mr[i].start;
				size = si->mr[i].size;
			}
		}

		if (size <= 0)
			panic("No suitable DRAM?\n");
		/*
		printf("Loading into memory region 0x%08X-0x%08X (%d MiB)\n",
		    start, start + size, size / 1024 / 1024);
		*/
	}
	if (o > size)
		panic("Address offset 0x%08jX bigger than size 0x%08X\n",
		      (intmax_t)o, size);
	return (void *)(start + o);
}

ssize_t
uboot_copyin(const void *src, vm_offset_t dest, const size_t len)
{
	bcopy(src, uboot_vm_translate(dest), len);
	return (len);
}

ssize_t
uboot_copyout(const vm_offset_t src, void *dest, const size_t len)
{
	bcopy(uboot_vm_translate(src), dest, len);
	return (len);
}

ssize_t
uboot_readin(const int fd, vm_offset_t dest, const size_t len)
{
	return (read(fd, uboot_vm_translate(dest), len));
}