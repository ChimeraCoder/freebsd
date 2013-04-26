
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

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)pty.c	8.3 (Berkeley) 5/16/94";
#endif
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <libutil.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int
openpty(int *amaster, int *aslave, char *name, struct termios *termp,
    struct winsize *winp)
{
	const char *slavename;
	int master, slave;

	master = posix_openpt(O_RDWR|O_NOCTTY);
	if (master == -1)
		return (-1);

	if (grantpt(master) == -1)
		goto bad;

	if (unlockpt(master) == -1)
		goto bad;

	slavename = ptsname(master);
	if (slavename == NULL)
		goto bad;

	slave = open(slavename, O_RDWR);
	if (slave == -1)
		goto bad;

	*amaster = master;
	*aslave = slave;

	if (name)
		strcpy(name, slavename);
	if (termp)
		tcsetattr(slave, TCSAFLUSH, termp);
	if (winp)
		ioctl(slave, TIOCSWINSZ, (char *)winp);

	return (0);

bad:	close(master);
	return (-1);
}

int
forkpty(int *amaster, char *name, struct termios *termp, struct winsize *winp)
{
	int master, slave, pid;

	if (openpty(&master, &slave, name, termp, winp) == -1)
		return (-1);
	switch (pid = fork()) {
	case -1:
		return (-1);
	case 0:
		/*
		 * child
		 */
		(void) close(master);
		login_tty(slave);
		return (0);
	}
	/*
	 * parent
	 */
	*amaster = master;
	(void) close(slave);
	return (pid);
}