
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
#ifndef lint
__FBSDID("$FreeBSD$");
#endif /* not lint */

#include "namespace.h"
#include <sys/param.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <paths.h>
#include <stdlib.h>
#include "un-namespace.h"

/*
 * __isptmaster():  return whether the file descriptor refers to a
 *                  pseudo-terminal master device.
 */
static int
__isptmaster(int fildes)
{

	if (_ioctl(fildes, TIOCPTMASTER) == 0)
		return (0);

	if (errno != EBADF)
		errno = EINVAL;

	return (-1);
}

/*
 * In our implementation, grantpt() and unlockpt() don't actually have
 * any use, because PTY's are created on the fly and already have proper
 * permissions upon creation.
 *
 * Just make sure `fildes' actually points to a real PTY master device.
 */
__strong_reference(__isptmaster, grantpt);
__strong_reference(__isptmaster, unlockpt);

/*
 * ptsname():  return the pathname of the slave pseudo-terminal device
 *             associated with the specified master.
 */
char *
ptsname(int fildes)
{
	static char pt_slave[sizeof _PATH_DEV + SPECNAMELEN] = _PATH_DEV;
	char *ret = NULL;

	/* Make sure fildes points to a master device. */
	if (__isptmaster(fildes) != 0)
		goto done;

	if (fdevname_r(fildes, pt_slave + (sizeof _PATH_DEV - 1),
	    sizeof pt_slave - (sizeof _PATH_DEV - 1)) != NULL)
		ret = pt_slave;

done:
	return (ret);
}