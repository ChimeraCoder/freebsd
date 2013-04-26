
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

#include "libuserboot.h"

ssize_t
userboot_copyin(const void *src, vm_offset_t va, size_t len)
{

        CALLBACK(copyin, src, va, len);
	return (len);
}

ssize_t
userboot_copyout(vm_offset_t va, void *dst, size_t len)
{

	CALLBACK(copyout, va, dst, len);
	return (len);
}

ssize_t
userboot_readin(int fd, vm_offset_t va, size_t len)
{
	ssize_t res, s;
	size_t sz;
	char buf[4096];

	res = 0;
	while (len > 0) {
		sz = len;
		if (sz > sizeof(buf))
			sz = sizeof(buf);
		s = read(fd, buf, sz);
		if (s == 0)
			break;
		if (s < 0)
			return (s);
		CALLBACK(copyin, buf, va, s);
		len -= s;
		res += s;
		va += s;
	}
	return (res);
}