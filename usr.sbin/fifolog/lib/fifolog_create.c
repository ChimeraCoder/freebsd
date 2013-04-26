
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/endian.h>
#include <sys/stat.h>
#include <sys/disk.h>

#include "fifolog.h"
#include "libfifolog.h"

const char *
fifolog_create(const char *fn, off_t size, ssize_t recsize)
{
	int i, fd;
	ssize_t u;
	off_t ms;
	struct stat st;
	char *buf;
	int created;

	fd = open(fn, O_WRONLY | O_TRUNC | O_EXCL | O_CREAT, 0644);
	if (fd < 0) {
		created = 0;
		fd = open(fn, O_WRONLY);
		if (fd < 0)
			return ("Could not open");
	} else
		created = 1;

	/* Default sectorsize is 512 */
	if (recsize == 0)
		recsize = 512;

	/* See what we got... */
	i = fstat(fd, &st);
	assert(i == 0);
	if (!S_ISBLK(st.st_mode) &&
	    !S_ISCHR(st.st_mode) &&
	    !S_ISREG(st.st_mode)) {
		assert(!close (fd));
		return ("Wrong file type");
	}

	if(!created && S_ISREG(st.st_mode)) {
		assert(!close (fd));
		return ("Wrong file type");
	}

	/* For raw disk with larger sectors: use 1 sector */
	i = ioctl(fd, DIOCGSECTORSIZE, &u);
	if (i == 0 && (u > recsize || (recsize % u) != 0))
		recsize = u;

	/* If no configured size, or too large for disk, use device size */
	i = ioctl(fd, DIOCGMEDIASIZE, &ms);
	if (i == 0 && (size == 0 || size > ms))
		size = ms;

	if (size == 0 && S_ISREG(st.st_mode))
		size = st.st_size;

	if (size == 0)
		size = recsize * (off_t)(24*60*60);

	if (S_ISREG(st.st_mode) && ftruncate(fd, size) < 0)
		return ("Could not ftrunc");

	buf = calloc(recsize, 1);
	if (buf == NULL)
		return ("Could not malloc");

	strcpy(buf, FIFOLOG_FMT_MAGIC);		/*lint !e64 */
	be32enc(buf + FIFOLOG_OFF_BS, recsize);
	if (recsize != pwrite(fd, buf, recsize, 0)) {
		i = errno;
		free(buf);
		errno = i;
		return ("Could not write first sector");
	}
	memset(buf, 0, recsize);
	if ((int)recsize != pwrite(fd, buf, recsize, recsize)) {
		i = errno;
		free(buf);
		errno = i;
		return ("Could not write second sector");
	}
	free(buf);
	assert(0 == close(fd));
	return (NULL);
}