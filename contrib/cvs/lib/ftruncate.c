
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
/* ftruncate emulations that work on some System V's.   This file is in the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <fcntl.h>

#ifdef F_CHSIZE
int
ftruncate (fd, length)
     int fd;
     off_t length;
{
  return fcntl (fd, F_CHSIZE, length);
}
#else
#ifdef F_FREESP
/* The following function was written by
   kucharsk@Solbourne.com (William Kucharski) */

#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

int
ftruncate (fd, length)
     int fd;
     off_t length;
{
  struct flock fl;
  struct stat filebuf;

  if (fstat (fd, &filebuf) < 0)
    return -1;

  if (filebuf.st_size < length)
    {
      /* Extend file length. */
      if (lseek (fd, (length - 1), SEEK_SET) < 0)
	return -1;

      /* Write a "0" byte. */
      if (write (fd, "", 1) != 1)
	return -1;
    }
  else
    {
      /* Truncate length. */
      fl.l_whence = 0;
      fl.l_len = 0;
      fl.l_start = length;
      fl.l_type = F_WRLCK;	/* Write lock on file space. */

      /* This relies on the UNDOCUMENTED F_FREESP argument to
	 fcntl, which truncates the file so that it ends at the
	 position indicated by fl.l_start.
	 Will minor miracles never cease? */
      if (fcntl (fd, F_FREESP, &fl) < 0)
	return -1;
    }

  return 0;
}
#else
int
ftruncate (fd, length)
     int fd;
     off_t length;
{
  return chsize (fd, length);
}
#endif
#endif