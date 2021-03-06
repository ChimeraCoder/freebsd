
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
static char sccsid[] = "@(#)lseek.c	8.1 (Berkeley) 6/17/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "libc_private.h"

/*
 * This function provides 64-bit offset padding that
 * is not supplied by GCC 1.X but is supplied by GCC 2.X.
 */
off_t
lseek(fd, offset, whence)
	int	fd;
	off_t	offset;
	int	whence;
{

	if (__getosreldate() >= 700051)
		return(__sys_lseek(fd, offset, whence));
	else
		return(__sys_freebsd6_lseek(fd, 0, offset, whence));
}