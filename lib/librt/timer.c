
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
#include <sys/syscall.h>

#include "namespace.h"
#include <errno.h>
#include <stddef.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include "sigev_thread.h"
#include "un-namespace.h"

extern int __sys_ktimer_create(clockid_t, struct sigevent *__restrict,
	int *__restrict);
extern int __sys_ktimer_delete(int);
extern int __sys_ktimer_gettime(int, struct itimerspec *);
extern int __sys_ktimer_getoverrun(int);
extern int __sys_ktimer_settime(int, int,
	const struct itimerspec *__restrict, struct itimerspec *__restrict);

struct __timer {
	int oshandle;
	struct sigev_node *node;
};

__weak_reference(__timer_create, timer_create);
__weak_reference(__timer_create, _timer_create);
__weak_reference(__timer_delete, timer_delete);
__weak_reference(__timer_delete, _timer_delete);
__weak_reference(__timer_gettime, timer_gettime);
__weak_reference(__timer_gettime, _timer_gettime);
__weak_reference(__timer_settime, timer_settime);
__weak_reference(__timer_settime, _timer_settime);
__weak_reference(__timer_getoverrun, timer_getoverrun);
__weak_reference(__timer_getoverrun, _timer_getoverrun);

typedef void (*timer_func)(union sigval val, int overrun);

static void
timer_dispatch(struct sigev_node *sn)
{
	timer_func f = sn->sn_func;

	/* I want to avoid expired notification. */
	if (sn->sn_info.si_value.sival_int == sn->sn_gen)
		f(sn->sn_value, sn->sn_info.si_overrun);
}

int
__timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid)
{
	struct __timer *timer;
	struct sigevent ev;
	struct sigev_node *sn;
	int ret, err;

	timer = malloc(sizeof(struct __timer));
	if (timer == NULL)
		return (-1);

	if (evp == NULL || evp->sigev_notify != SIGEV_THREAD) {
		ret = __sys_ktimer_create(clockid, evp, &timer->oshandle);
		if (ret == -1) {
			err = errno;
			free(timer);
			errno = err;
			return (ret);
		}
		timer->node = NULL;
		*timerid = timer;
		return (0);
	}

	if (__sigev_check_init()) {
		free(timer);
		errno = EINVAL;
		return (-1);
	}

	sn = __sigev_alloc(SI_TIMER, evp, NULL, 0);
	if (sn == NULL) {
		free(timer);
		errno = EAGAIN;
		return (-1);
	}

	__sigev_get_sigevent(sn, &ev, sn->sn_gen);
	ret = __sys_ktimer_create(clockid, &ev, &timer->oshandle);
	if (ret != 0) {
		err = errno;
		__sigev_free(sn);
		free(timer);
		errno = err;
		return (-1);
	}
	sn->sn_flags |= SNF_SYNC;
	sn->sn_dispatch = timer_dispatch;
	sn->sn_id = timer->oshandle;
	timer->node = sn;
	__sigev_list_lock();
	__sigev_register(sn);
	__sigev_list_unlock();
	*timerid = timer;
	return (0);
}

int
__timer_delete(timer_t timerid)
{
	int ret, err;

	if (timerid->node != NULL) {
		__sigev_list_lock();
		__sigev_delete_node(timerid->node);
		__sigev_list_unlock();
	}
	ret = __sys_ktimer_delete(timerid->oshandle);
	err = errno;
	free(timerid);
	errno = err;
	return (ret);	
}

int
__timer_gettime(timer_t timerid, struct itimerspec *value)
{

	return __sys_ktimer_gettime(timerid->oshandle, value);
}

int
__timer_getoverrun(timer_t timerid)
{

	return __sys_ktimer_getoverrun(timerid->oshandle);
}

int
__timer_settime(timer_t timerid, int flags,
	const struct itimerspec *__restrict value,
	struct itimerspec *__restrict ovalue)
{

	return __sys_ktimer_settime(timerid->oshandle,
		flags, value, ovalue);
}

int
__timer_oshandle(timer_t timerid)
{

	return (timerid->oshandle);
}