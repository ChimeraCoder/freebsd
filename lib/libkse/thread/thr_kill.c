
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
#include <signal.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_kill, pthread_kill);

int
_pthread_kill(pthread_t pthread, int sig)
{
	struct pthread *curthread = _get_curthread();
	int ret;

	/* Check for invalid signal numbers: */
	if (sig < 0 || sig > _SIG_MAXSIG)
		/* Invalid signal: */
		ret = EINVAL;
	/*
	 * Ensure the thread is in the list of active threads, and the
	 * signal is valid (signal 0 specifies error checking only) and
	 * not being ignored:
	 */
	else if ((ret = _thr_ref_add(curthread, pthread, /*include dead*/0))
	    == 0) {
		if ((sig > 0) &&
		    (_thread_sigact[sig - 1].sa_handler != SIG_IGN))
			_thr_sig_send(pthread, sig);
		_thr_ref_delete(curthread, pthread);
	}

	/* Return the completion status: */
	return (ret);
}