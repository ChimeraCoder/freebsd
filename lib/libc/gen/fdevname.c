
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

#include "namespace.h"
#include <sys/param.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "un-namespace.h"

char *
fdevname_r(int fd, char *buf, int len)
{
	struct fiodgname_arg fgn;

	fgn.buf = buf;
	fgn.len = len;

	if (_ioctl(fd, FIODGNAME, &fgn) == -1)
		return (NULL);
	return (buf);
}

char *
fdevname(int fd)
{
	static char buf[SPECNAMELEN + 1];

	return (fdevname_r(fd, buf, sizeof(buf)));
}