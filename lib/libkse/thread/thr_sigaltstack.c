
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
#include <errno.h>
#include <signal.h>
#include "un-namespace.h"
#include "thr_private.h"

int	_sigaltstack(stack_t *_ss, stack_t *_oss);

__weak_reference(_sigaltstack, sigaltstack);

int
_sigaltstack(stack_t *_ss, stack_t *_oss)
{
	struct pthread *curthread = _get_curthread();
	stack_t ss, oss;
	int oonstack, errsave, ret;
	kse_critical_t crit;

	if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM) {
		crit = _kse_critical_enter();
		ret = __sys_sigaltstack(_ss, _oss);
		errsave = errno;
		/* Get a copy */
		if (ret == 0 && _ss != NULL)
			curthread->sigstk = *_ss;
		_kse_critical_leave(crit);
		errno = errsave;
		return (ret);
	}

	if (_ss)
		ss = *_ss;
	if (_oss)
		oss = *_oss;

	/* Should get and set stack in atomic way */
	crit = _kse_critical_enter();
	oonstack = _thr_sigonstack(&ss);
	if (_oss != NULL) {
		oss = curthread->sigstk;
		oss.ss_flags = (curthread->sigstk.ss_flags & SS_DISABLE)
		    ? SS_DISABLE : ((oonstack) ? SS_ONSTACK : 0);
	}

	if (_ss != NULL) {
		if (oonstack) {
			_kse_critical_leave(crit);
			errno = EPERM;
			return (-1);
		}
		if ((ss.ss_flags & ~SS_DISABLE) != 0) {
			_kse_critical_leave(crit);
			errno = EINVAL;
			return (-1);
		}
		if (!(ss.ss_flags & SS_DISABLE)) {
			if (ss.ss_size < MINSIGSTKSZ) {
				_kse_critical_leave(crit);
				errno = ENOMEM;
				return (-1);
			}
			curthread->sigstk = ss;
		} else {
			curthread->sigstk.ss_flags |= SS_DISABLE;
		}
	}
	_kse_critical_leave(crit);
	if (_oss != NULL)
		*_oss = oss;
	return (0);
}

int
_thr_sigonstack(void *sp)
{
	struct pthread *curthread = _get_curthread();

	return ((curthread->sigstk.ss_flags & SS_DISABLE) == 0 ?
	    (((size_t)sp - (size_t)curthread->sigstk.ss_sp) < curthread->sigstk.ss_size)
	    : 0);
}