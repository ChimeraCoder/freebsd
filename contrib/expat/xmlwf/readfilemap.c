
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
   See the file COPYING for copying permission.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __WATCOMC__
#ifndef __LINUX__
#include <io.h>
#else
#include <unistd.h>
#endif
#endif

#ifdef __BEOS__
#include <unistd.h>
#endif

#ifndef S_ISREG
#ifndef S_IFREG
#define S_IFREG _S_IFREG
#endif
#ifndef S_IFMT
#define S_IFMT _S_IFMT
#endif
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif /* not S_ISREG */

#ifndef O_BINARY
#ifdef _O_BINARY
#define O_BINARY _O_BINARY
#else
#define O_BINARY 0
#endif
#endif

#include "filemap.h"

int
filemap(const char *name,
        void (*processor)(const void *, size_t, const char *, void *arg),
        void *arg)
{
  size_t nbytes;
  int fd;
  int n;
  struct stat sb;
  void *p;

  fd = open(name, O_RDONLY|O_BINARY);
  if (fd < 0) {
    perror(name);
    return 0;
  }
  if (fstat(fd, &sb) < 0) {
    perror(name);
    close(fd);
    return 0;
  }
  if (!S_ISREG(sb.st_mode)) {
    fprintf(stderr, "%s: not a regular file\n", name);
    close(fd);
    return 0;
  }
  nbytes = sb.st_size;
  /* malloc will return NULL with nbytes == 0, handle files with size 0 */
  if (nbytes == 0) {
    static const char c = '\0';
    processor(&c, 0, name, arg);
    close(fd);
    return 1;
  }
  p = malloc(nbytes);
  if (!p) {
    fprintf(stderr, "%s: out of memory\n", name);
    close(fd);
    return 0;
  }
  n = read(fd, p, nbytes);
  if (n < 0) {
    perror(name);
    free(p);
    close(fd);
    return 0;
  }
  if (n != nbytes) {
    fprintf(stderr, "%s: read unexpected number of bytes\n", name);
    free(p);
    close(fd);
    return 0;
  }
  processor(p, nbytes, name, arg);
  free(p);
  close(fd);
  return 1;
}