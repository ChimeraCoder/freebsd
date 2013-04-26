
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
#include "libzfs.h"

__attribute__((weak))
int
zfs_parsedev(struct zfs_devdesc *dev, const char *devspec, const char **path)
{
	return (EINVAL);
}

__attribute__((weak))
char *
zfs_fmtdev(void *vdev)
{
    static char	buf[128];

    return (buf);
}