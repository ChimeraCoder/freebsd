
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

#include "namespace.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "un-namespace.h"

int
__waitid(idtype_t idtype, id_t id, siginfo_t *info, int flags)
{
	int status;
	pid_t ret;

	ret = _wait6(idtype, id, &status, flags, NULL, info);

	/*
	 * According to SUSv4, waitid() shall not return a PID when a
	 * process is found, but only 0.  If a process was actually
	 * found, siginfo_t fields si_signo and si_pid will be
	 * non-zero.  In case WNOHANG was set in the flags and no
	 * process was found those fields are set to zero using
	 * memset() below.
	 */
	if (ret == 0 && info != NULL)
		memset(info, 0, sizeof(*info));
	else if (ret > 0)
		ret = 0;
	return (ret);
}

__weak_reference(__waitid, waitid);
__weak_reference(__waitid, _waitid);