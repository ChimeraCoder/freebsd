
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

/*
 * MD primitives supporting placement of module data 
 *
 * XXX should check load address/size against memory top.
 */
#include <stand.h>

#include "libi386.h"
#include "btxv86.h"

ssize_t
i386_copyin(const void *src, vm_offset_t dest, const size_t len)
{
    bcopy(src, PTOV(dest), len);
    return(len);
}

ssize_t
i386_copyout(const vm_offset_t src, void *dest, const size_t len)
{
    bcopy(PTOV(src), dest, len);
    return(len);
}


ssize_t
i386_readin(const int fd, vm_offset_t dest, const size_t len)
{
    return (read(fd, PTOV(dest), len));
}