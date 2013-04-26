
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

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include "threads.h"

struct thrd_param {
	thrd_start_t	 func;
	void		*arg;
};

static void *
thrd_entry(void *arg)
{
	struct thrd_param tp;

	tp = *(struct thrd_param *)arg;
	free(arg);
	return ((void *)(intptr_t)tp.func(tp.arg));
}

int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	struct thrd_param *tp;

	/*
	 * Work around return type inconsistency.  Wrap execution using
	 * a function conforming to pthread_create()'s start_routine.
	 */
	tp = malloc(sizeof(*tp));
	if (tp == NULL)
		return (thrd_nomem);
	tp->func = func;
	tp->arg = arg;
	if (pthread_create(thr, NULL, thrd_entry, tp) != 0) {
		free(tp);
		return (thrd_error);
	}
	return (thrd_success);
}

thrd_t
thrd_current(void)
{

	return (pthread_self());
}

int
thrd_detach(thrd_t thr)
{

	if (pthread_detach(thr) != 0)
		return (thrd_error);
	return (thrd_success);
}

int
thrd_equal(thrd_t thr0, thrd_t thr1)
{

	return (pthread_equal(thr0, thr1));
}

_Noreturn void
thrd_exit(int res)
{

	pthread_exit((void *)(intptr_t)res);
}

int
thrd_join(thrd_t thr, int *res)
{
	void *value_ptr;

	if (pthread_join(thr, &value_ptr) != 0)
		return (thrd_error);
	*res = (intptr_t)value_ptr;
	return (thrd_success);
}

int
thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{

	return (nanosleep(duration, remaining));
}

void
thrd_yield(void)
{

	pthread_yield();
}