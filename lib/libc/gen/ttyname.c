
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ttyname.c	8.2 (Berkeley) 1/27/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <paths.h>
#include <errno.h>
#include "reentrant.h"
#include "un-namespace.h"

#include "libc_private.h"

static char ttyname_buf[sizeof(_PATH_DEV) + MAXNAMLEN];

static once_t		ttyname_init_once = ONCE_INITIALIZER;
static thread_key_t	ttyname_key;
static int		ttyname_keycreated = 0;

int
ttyname_r(int fd, char *buf, size_t len)
{
	size_t used;

	*buf = '\0';

	/* Must be a terminal. */
	if (!isatty(fd))
		return (ENOTTY);
	/* Must have enough room */
	if (len <= sizeof(_PATH_DEV))
		return (ERANGE);

	strcpy(buf, _PATH_DEV);
	used = strlen(buf);
	if (fdevname_r(fd, buf + used, len - used) == NULL)
		return (ENOTTY);
	return (0);
}

static void
ttyname_keycreate(void)
{
	ttyname_keycreated = (thr_keycreate(&ttyname_key, free) == 0);
}

char *
ttyname(int fd)
{
	char	*buf;

	if (thr_main() != 0)
		buf = ttyname_buf;
	else {
		if (thr_once(&ttyname_init_once, ttyname_keycreate) != 0 ||
		    !ttyname_keycreated)
			return (NULL);
		if ((buf = thr_getspecific(ttyname_key)) == NULL) {
			if ((buf = malloc(sizeof ttyname_buf)) == NULL)
				return (NULL);
			if (thr_setspecific(ttyname_key, buf) != 0) {
				free(buf);
				return (NULL);
			}
		}
	}

	if (ttyname_r(fd, buf, sizeof ttyname_buf) != 0)
		return (NULL);
	return (buf);
}