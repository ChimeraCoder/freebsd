
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

#include <err.h>
#include <string.h>
#include <unistd.h>

#include "kgzip.h"

/*
 * Close a file.
 */
void
xclose(const struct iodesc *id)
{
    if (close(id->fd))
	err(1, "%s", id->fname);
}

/*
 * Copy bytes from one file to another.
 */
void
xcopy(const struct iodesc * idi, const struct iodesc * ido,
      size_t nbyte, off_t offset)
{
    char buf[8192];
    size_t n;

    while (nbyte) {
	if ((n = sizeof(buf)) > nbyte)
	    n = nbyte;
	if (xread(idi, buf, n, offset) != n)
	    errx(1, "%s: Short read", idi->fname);
	xwrite(ido, buf, n);
	nbyte -= n;
	offset = -1;
    }
}

/*
 * Write binary zeroes to a file.
 */
void
xzero(const struct iodesc * id, size_t nbyte)
{
    char buf[8192];
    size_t n;

    memset(buf, 0, sizeof(buf));
    while (nbyte) {
	if ((n = sizeof(buf)) > nbyte)
	    n = nbyte;
	xwrite(id, buf, n);
	nbyte -= n;
    }
}

/*
 * Read from a file.
 */
size_t
xread(const struct iodesc * id, void *buf, size_t nbyte, off_t offset)
{
    ssize_t n;

    if (offset != -1 && lseek(id->fd, offset, SEEK_SET) != offset)
	err(1, "%s", id->fname);
    if ((n = read(id->fd, buf, nbyte)) == -1)
	err(1, "%s", id->fname);
    return (size_t)n;
}

/*
 * Write to a file.
 */
void
xwrite(const struct iodesc * id, const void *buf, size_t nbyte)
{
    ssize_t n;

    if ((n = write(id->fd, buf, nbyte)) == -1)
	err(1, "%s", id->fname);
    if ((size_t)n != nbyte)
	errx(1, "%s: Short write", id->fname);
}

/*
 * Reposition within a file.
 */
void
xseek(const struct iodesc *id, off_t offset)
{
    if (lseek(id->fd, offset, SEEK_SET) != offset)
	err(1, "%s", id->fname);
}