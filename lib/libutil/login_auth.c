
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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <libutil.h>
#include <limits.h>
#include <login_cap.h>
#include <paths.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>


/*
 * auth_checknologin()
 * Checks for the existance of a nologin file in the login_cap
 * capability <lc>.  If there isn't one specified, then it checks
 * to see if this class should just ignore nologin files.  Lastly,
 * it tries to print out the default nologin file, and, if such
 * exists, it exits.
 */

void
auth_checknologin(login_cap_t *lc)
{
  const char *file;

  /* Do we ignore a nologin file? */
  if (login_getcapbool(lc, "ignorenologin", 0))
    return;

  /* Note that <file> will be "" if there is no nologin capability */
  if ((file = login_getcapstr(lc, "nologin", "", NULL)) == NULL)
    exit(1);

  /*
   * *file is true IFF there was a "nologin" capability
   * Note that auth_cat() returns 1 only if the specified
   * file exists, and is readable.  E.g., /.nologin exists.
   */
  if ((*file && auth_cat(file)) || auth_cat(_PATH_NOLOGIN))
    exit(1);
}


/*
 * auth_cat()
 * Checks for the readability of <file>; if it can be opened for
 * reading, it prints it out to stdout, and then exits.  Otherwise,
 * it returns 0 (meaning no nologin file).
 */

int
auth_cat(const char *file)
{
  int fd, count;
  char buf[BUFSIZ];

  if ((fd = open(file, O_RDONLY)) < 0)
    return 0;
  while ((count = read(fd, buf, sizeof(buf))) > 0)
    (void)write(fileno(stdout), buf, count);
  close(fd);
  sleep(5);	/* wait an arbitrary time to drain */
  return 1;
}