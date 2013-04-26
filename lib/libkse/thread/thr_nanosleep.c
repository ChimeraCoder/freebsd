
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
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

int	__nanosleep(const struct timespec *time_to_sleep,
	    struct timespec *time_remaining);

__weak_reference(__nanosleep, nanosleep);

int
_nanosleep(const struct timespec *time_to_sleep,
    struct timespec *time_remaining)
{
	struct pthread	*curthread = _get_curthread();
	int             ret = 0;
	struct timespec ts, ts1;
	struct timespec remaining_time;
	struct timespec wakeup_time;

	/* Check if the time to sleep is legal: */
	if ((time_to_sleep == NULL) || (time_to_sleep->tv_sec < 0) ||
	    (time_to_sleep->tv_nsec < 0) ||
	    (time_to_sleep->tv_nsec >= 1000000000)) {
		/* Return an EINVAL error : */
		errno = EINVAL;
		ret = -1;
	} else {
		if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM)
			return (__sys_nanosleep(time_to_sleep, time_remaining));
			
		KSE_GET_TOD(curthread->kse, &ts);

		/* Calculate the time for the current thread to wake up: */
		TIMESPEC_ADD(&wakeup_time, &ts, time_to_sleep);

		THR_LOCK_SWITCH(curthread);
		curthread->interrupted = 0;
		curthread->wakeup_time = wakeup_time;
		THR_SET_STATE(curthread, PS_SLEEP_WAIT);

		/* Reschedule the current thread to sleep: */
		_thr_sched_switch_unlocked(curthread);

		/* Calculate the remaining time to sleep: */
		KSE_GET_TOD(curthread->kse, &ts1);
		remaining_time.tv_sec = time_to_sleep->tv_sec
		    + ts.tv_sec - ts1.tv_sec;
		remaining_time.tv_nsec = time_to_sleep->tv_nsec
		    + ts.tv_nsec - ts1.tv_nsec;

		/* Check if the nanosecond field has underflowed: */
		if (remaining_time.tv_nsec < 0) {
			/* Handle the underflow: */
			remaining_time.tv_sec -= 1;
			remaining_time.tv_nsec += 1000000000;
		}
		/* Check if the nanosecond field has overflowed: */
		else if (remaining_time.tv_nsec >= 1000000000) {
			/* Handle the overflow: */
			remaining_time.tv_sec += 1;
			remaining_time.tv_nsec -= 1000000000;
		}

		/* Check if the sleep was longer than the required time: */
		if (remaining_time.tv_sec < 0) {
			/* Reset the time left: */
			remaining_time.tv_sec = 0;
			remaining_time.tv_nsec = 0;
		}

		/* Check if the time remaining is to be returned: */
		if (time_remaining != NULL) {
			/* Return the actual time slept: */
			time_remaining->tv_sec = remaining_time.tv_sec;
			time_remaining->tv_nsec = remaining_time.tv_nsec;
		}

		/* Check if the sleep was interrupted: */
		if (curthread->interrupted) {
			/* Return an EINTR error : */
			errno = EINTR;
			ret = -1;
		}
	}
	return (ret);
}

int
__nanosleep(const struct timespec *time_to_sleep,
    struct timespec *time_remaining)
{
	struct pthread *curthread = _get_curthread();
	int		ret;

	_thr_cancel_enter(curthread);
	ret = _nanosleep(time_to_sleep, time_remaining);
	_thr_cancel_leave(curthread, 1);

	return (ret);
}