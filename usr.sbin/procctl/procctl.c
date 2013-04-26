
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
 * procctl -- clear the event mask, and continue, any specified processes.
 * This is largely an example of how to use the procfs interface; however,
 * for now, it is also sometimes necessary, as a stopped process will not
 * otherwise continue.  (This will be fixed in a later version of the
 * procfs code, almost certainly; however, this program will still be useful
 * for some annoying circumstances.)
 */

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/pioctl.h>

int
main(int ac, char **av) {
  int fd;
  int i;

  for (i = 1; i < ac; i++) {
    char buf[32];

    snprintf(buf, sizeof(buf), "/proc/%s/mem", av[i]);
    fd = open(buf, O_RDWR);
    if (fd == -1) {
      if (errno == ENOENT)
	continue;
      warn("cannot open pid %s", av[i]);
      continue;
    }
    if (ioctl(fd, PIOCBIC, ~0) == -1)
      warn("cannot clear process %s's event mask", av[i]);
    if (ioctl(fd, PIOCCONT, 0) == -1 && errno != EINVAL)
      warn("cannot continue process %s", av[i]);
    close(fd);
  }
  return 0;
}