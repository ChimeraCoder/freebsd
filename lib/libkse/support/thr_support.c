
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
#include <sys/types.h>
#include <sys/time.h>
#include <sys/kse.h>
#include <signal.h>
#include <string.h>

#include "thr_private.h"

__strong_reference(clock_gettime, _thr_clock_gettime);
__strong_reference(kse_exit, _thr_kse_exit);
__strong_reference(kse_wakeup, _thr_kse_wakeup);
__strong_reference(kse_create, _thr_kse_create);
__strong_reference(kse_thr_interrupt, _thr_kse_thr_interrupt);
__strong_reference(kse_release, _thr_kse_release);
__strong_reference(kse_switchin, _thr_kse_switchin);

__strong_reference(sigaction, _thr_sigaction);
__strong_reference(sigprocmask, _thr_sigprocmask);
__strong_reference(sigemptyset, _thr_sigemptyset);
__strong_reference(sigaddset, _thr_sigaddset);
__strong_reference(sigfillset, _thr_sigfillset);
__strong_reference(sigismember, _thr_sigismember);
__strong_reference(sigdelset, _thr_sigdelset);

__strong_reference(memset, _thr_memset);
__strong_reference(memcpy, _thr_memcpy);
__strong_reference(strcpy, _thr_strcpy);
__strong_reference(strlen, _thr_strlen);
__strong_reference(bzero, _thr_bzero);
__strong_reference(bcopy, _thr_bcopy);

__strong_reference(__sys_write, _thr__sys_write);
__strong_reference(__sys_sigtimedwait, _thr__sys_sigtimedwait);