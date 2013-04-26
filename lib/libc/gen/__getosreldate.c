
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
#include <sys/sysctl.h>
#include <errno.h>
#include <link.h>
#include "libc_private.h"

int __getosreldate(void);

/*
 * This is private to libc.  It is intended for wrapping syscall stubs in order
 * to avoid having to put SIGSYS signal handlers in place to test for presence
 * of new syscalls.  This caches the result in order to be as quick as possible.
 *
 * Use getosreldate(3) for public use as it respects the $OSVERSION environment
 * variable.
 */

int
__getosreldate(void)
{
	static int osreldate;
	size_t len;
	int oid[2];
	int error, osrel;

	if (osreldate != 0)
		return (osreldate);

	error = _elf_aux_info(AT_OSRELDATE, &osreldate, sizeof(osreldate));
	if (error == 0 && osreldate != 0)
		return (osreldate);

	oid[0] = CTL_KERN;
	oid[1] = KERN_OSRELDATE;
	osrel = 0;
	len = sizeof(osrel);
	error = sysctl(oid, 2, &osrel, &len, NULL, 0);
	if (error == 0 && osrel > 0 && len == sizeof(osrel))
		osreldate = osrel;
	return (osreldate);
}