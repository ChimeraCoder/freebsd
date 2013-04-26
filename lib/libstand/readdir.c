
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
#include "stand.h"

struct dirent *
readdirfd(int fd)
{
	static struct dirent dir;		/* XXX not thread safe */
	struct open_file *f = &files[fd];

	if ((unsigned)fd >= SOPEN_MAX || !(f->f_flags & F_READ)) {
		errno = EBADF;
		return (NULL);
	}
	if (f->f_flags & F_RAW) {
		errno = EIO;
		return (NULL);
	}
	errno = (f->f_ops->fo_readdir)(f, &dir);
	if (errno)
		return (NULL);
	return (&dir);
}