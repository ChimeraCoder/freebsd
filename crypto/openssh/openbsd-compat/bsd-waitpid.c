
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

#include "includes.h"

#ifndef HAVE_WAITPID 
#include <errno.h>
#include <sys/wait.h>
#include "bsd-waitpid.h"

pid_t
waitpid(int pid, int *stat_loc, int options)
{
	union wait statusp;
	pid_t wait_pid;

	if (pid <= 0) {
		if (pid != -1) {
			errno = EINVAL;
			return (-1);
		}
		/* wait4() wants pid=0 for indiscriminate wait. */
		pid = 0;
	}
        wait_pid = wait4(pid, &statusp, options, NULL);
	if (stat_loc)
        	*stat_loc = (int) statusp.w_status;            

        return (wait_pid);                               
}

#endif /* !HAVE_WAITPID */