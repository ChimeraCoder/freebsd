
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

#include <dlfcn.h>
#include <stddef.h>
#include <unistd.h>

#include "libc_private.h"

/*
 * Whether we want to restrict dlopen()s.
 */
static int __libc_restricted_mode = 0;

void *
libc_dlopen(const char *path, int mode)
{

	if (__libc_restricted_mode) {
		_rtld_error("Service unavailable -- libc in restricted mode");
		return (NULL);
	} else
		return (dlopen(path, mode));
}

void
__FreeBSD_libc_enter_restricted_mode(void)
{

	__libc_restricted_mode = 1;
	return;
}