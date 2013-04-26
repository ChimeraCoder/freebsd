
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

#ifndef lint
static const char sccsid[] = "@(#)misc.c	8.1 (Berkeley) 6/9/93";
#endif

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "extern.h"

void
cat(char *file)
{
	register int fd, nr, nw;
	char buf[1024];

	if ((fd = open(file, O_RDONLY, 0)) < 0)
		err(1, "%s", file);

	while ((nr = read(fd, buf, sizeof(buf))) > 0)
		if ((nw = write(STDERR_FILENO, buf, nr)) == -1)
			err(1, "write to stderr");
	if (nr != 0)
		err(1, "%s", file);
	(void)close(fd);
}

int
outc(int c)
{
	return putc(c, stderr);
}