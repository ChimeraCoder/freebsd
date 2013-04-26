
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

#include "namespace.h"
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_execve, execve);

int 
_execve(const char *name, char *const *argv, char *const *envp)
{
	struct kse_execve_args args;
	struct pthread *curthread = _get_curthread();
	int ret;

	if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM)
		ret = __sys_execve(name, argv, envp);
	else {
		/*
		 * When exec'ing, set the kernel signal mask to the thread's
	 	 * signal mask to satisfy POSIX requirements.
		 */
		args.sigmask = curthread->sigmask;
		args.sigpend = curthread->sigpend;
		args.path = (char *)name;
		args.argv = (char **)argv;
		args.envp = (char **)envp;
		args.reserved = NULL;
		ret = kse_thr_interrupt(NULL, KSE_INTR_EXECVE, (long)&args);
	}

	return (ret);
}