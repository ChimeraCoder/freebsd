
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <spinlock.h>
#include <sys/signalvar.h>
#include "un-namespace.h"

#include "libc_private.h"
#include "thr_private.h"

pid_t	_fork(void);

__weak_reference(_fork, fork);

pid_t
_fork(void)
{
	sigset_t sigset, oldset;
	struct pthread *curthread;
	struct pthread_atfork *af;
	pid_t ret;
	int errsave;

	curthread = _get_curthread();

	if (!_kse_isthreaded()) {
		SIGFILLSET(sigset);
		__sys_sigprocmask(SIG_SETMASK, &sigset, &oldset);
		ret = __sys_fork();
		if (ret == 0)
			/* Child */
			__sys_sigprocmask(SIG_SETMASK, &curthread->sigmask,
			    NULL);
		else
			__sys_sigprocmask(SIG_SETMASK, &oldset, NULL);
		return (ret);
	}

	/*
	 * Masks all signals until we reach a safe point in
	 * _kse_single_thread, and the signal masks will be
	 * restored in that function, for M:N thread, all 
	 * signals were already masked in kernel atomically,
	 * we only need to do this for bound thread.
	 */
	if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM) {
		SIGFILLSET(sigset);
		__sys_sigprocmask(SIG_SETMASK, &sigset, &oldset);
	}

	_pthread_mutex_lock(&_thr_atfork_mutex);

	/* Run down atfork prepare handlers. */
	TAILQ_FOREACH_REVERSE(af, &_thr_atfork_list, atfork_head, qe) {
		if (af->prepare != NULL)
			af->prepare();
	}

	/* Fork a new process: */
	if (_kse_isthreaded() != 0) {
		_malloc_prefork();
	}
	if ((ret = __sys_fork()) == 0) {
		/* Child process */
		errsave = errno; 

		/* Kernel signal mask is restored in _kse_single_thread */
		_kse_single_thread(curthread);

		/* Run down atfork child handlers. */
		TAILQ_FOREACH(af, &_thr_atfork_list, qe) {
			if (af->child != NULL)
				af->child();
		}
		_thr_mutex_reinit(&_thr_atfork_mutex);
	} else {
		if (_kse_isthreaded() != 0) {
			_malloc_postfork();
		}
		errsave = errno; 
		if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM) {
			__sys_sigprocmask(SIG_SETMASK, &oldset, NULL);
		}
		/* Run down atfork parent handlers. */
		TAILQ_FOREACH(af, &_thr_atfork_list, qe) {
			if (af->parent != NULL)
				af->parent();
		}
		_pthread_mutex_unlock(&_thr_atfork_mutex);
	}
	errno = errsave;

	/* Return the process ID: */
	return (ret);
}