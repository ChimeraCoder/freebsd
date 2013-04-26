
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
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_sigaction, sigaction);

int
_sigaction(int sig, const struct sigaction * act, struct sigaction * oact)
{
	int ret = 0;
	int err = 0;
	struct sigaction newact, oldact;
	struct pthread *curthread;
	kse_critical_t crit;

	/* Check if the signal number is out of range: */
	if (sig < 1 || sig > _SIG_MAXSIG) {
		/* Return an invalid argument: */
		errno = EINVAL;
		ret = -1;
	} else {
		if (act)
			newact = *act;

		crit = _kse_critical_enter();
		curthread = _get_curthread();
		KSE_LOCK_ACQUIRE(curthread->kse, &_thread_signal_lock);

		oldact = _thread_sigact[sig - 1];

		/* Check if a signal action was supplied: */
		if (act != NULL) {
			/* Set the new signal handler: */
			_thread_sigact[sig - 1] = newact;
		}

		/*
		 * Check if the kernel needs to be advised of a change
		 * in signal action:
		 */
		if (act != NULL) {

			newact.sa_flags |= SA_SIGINFO;

			/*
			 * Check if the signal handler is being set to
			 * the default or ignore handlers:
			 */
			if (newact.sa_handler != SIG_DFL &&
			    newact.sa_handler != SIG_IGN) {
				/*
				 * Specify the thread kernel signal
				 * handler:
				 */
				newact.sa_sigaction = _thr_sig_handler;
			}
			/*
			 * Install libpthread signal handler wrapper
			 * for SIGINFO signal if threads dump enabled
			 * even if a user set the signal handler to
			 * SIG_DFL or SIG_IGN.
			 */
			if (sig == SIGINFO && _thr_dump_enabled()) {
				newact.sa_sigaction = _thr_sig_handler;
			}
			/* Change the signal action in the kernel: */
			if (__sys_sigaction(sig, &newact, NULL) != 0) {
				_thread_sigact[sig - 1] = oldact;
				/* errno is in kse, will copy it to thread */
				err = errno;
				ret = -1;
			}
		}
		KSE_LOCK_RELEASE(curthread->kse, &_thread_signal_lock);
		_kse_critical_leave(crit);
		/*
		 * Check if the existing signal action structure contents are
		 * to be returned: 
		*/
		if (oact != NULL) {
			/* Return the existing signal action contents: */
			*oact = oldact;
		}
		if (ret != 0) {
			/* Return errno to thread */
			errno = err;
		}
	}

	/* Return the completion status: */
	return (ret);
}