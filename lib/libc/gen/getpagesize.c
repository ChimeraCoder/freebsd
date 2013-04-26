
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getpagesize.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/sysctl.h>

#include <errno.h>
#include <link.h>
#include <unistd.h>

#include "libc_private.h"

/*
 * This is unlikely to change over the running time of any
 * program, so we cache the result to save some syscalls.
 *
 * NB: This function may be called from malloc(3) at initialization
 * NB: so must not result in a malloc(3) related call!
 */

int
getpagesize()
{
	int mib[2]; 
	static int value;
	size_t size;
	int error;

	if (value != 0)
		return (value);

	error = _elf_aux_info(AT_PAGESZ, &value, sizeof(value));
	if (error == 0 && value != 0)
		return (value);

	mib[0] = CTL_HW;
	mib[1] = HW_PAGESIZE;
	size = sizeof value;
	if (sysctl(mib, 2, &value, &size, NULL, 0) == -1)
		return (-1);

	return (value);
}