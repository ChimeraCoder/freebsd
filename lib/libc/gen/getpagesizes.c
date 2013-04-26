
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
#include <sys/mman.h>
#include <sys/sysctl.h>

#include <errno.h>
#include <link.h>

#include "libc_private.h"

/*
 * Retrieves page size information from the system.  Specifically, returns the
 * number of distinct page sizes that are supported by the system, if
 * "pagesize" is NULL and "nelem" is 0.  Otherwise, assigns up to "nelem" of
 * the system-supported page sizes to consecutive elements of the array
 * referenced by "pagesize", and returns the number of such page sizes that it
 * assigned to the array.  These page sizes are expressed in bytes.
 *
 * The implementation of this function does not directly or indirectly call
 * malloc(3) or any other dynamic memory allocator that may itself call this
 * function.
 */
int
getpagesizes(size_t pagesize[], int nelem)
{
	static u_long ps[MAXPAGESIZES];
	static int nops;
	size_t size;
	int error, i;

	if (nelem < 0 || (nelem > 0 && pagesize == NULL)) {
		errno = EINVAL;
		return (-1);
	}
	/* Cache the result of the sysctl(2). */
	if (nops == 0) {
		error = _elf_aux_info(AT_PAGESIZES, ps, sizeof(ps));
		size = sizeof(ps);
		if (error != 0 || ps[0] == 0) {
			if (sysctlbyname("hw.pagesizes", ps, &size, NULL, 0)
			    == -1)
				return (-1);
		}
		/* Count the number of page sizes that are supported. */
		nops = size / sizeof(ps[0]);
		while (nops > 0 && ps[nops - 1] == 0)
			nops--;
	}
	if (pagesize == NULL)
		return (nops);
	/* Return up to "nelem" page sizes from the cached result. */
	if (nelem > nops)
		nelem = nops;
	for (i = 0; i < nelem; i++)
		pagesize[i] = ps[i];
	return (nelem);
}