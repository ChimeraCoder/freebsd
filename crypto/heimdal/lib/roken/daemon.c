
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
static char sccsid[] = "@(#)daemon.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <config.h>

#ifndef HAVE_DAEMON

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "roken.h"

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
daemon(int nochdir, int noclose)
{
    int fd;

    switch (fork()) {
    case -1:
	return (-1);
    case 0:
	break;
    default:
	_exit(0);
    }

    if (setsid() == -1)
	return (-1);

    if (!nochdir)
	chdir("/");

    if (!noclose && (fd = open(_PATH_DEVNULL, O_RDWR, 0)) != -1) {
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	if (fd > 2)
	    close (fd);
    }
    return (0);
}

#endif /* HAVE_DAEMON */