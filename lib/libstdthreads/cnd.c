
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

#include <errno.h>
#include <pthread.h>

#include "threads.h"

int
cnd_broadcast(cnd_t *cond)
{

	if (pthread_cond_broadcast(cond) != 0)
		return (thrd_error);
	return (thrd_success);
}

void
cnd_destroy(cnd_t *cond)
{

	(void)pthread_cond_destroy(cond);
}

int
cnd_init(cnd_t *cond)
{

	switch (pthread_cond_init(cond, NULL)) {
	case 0:
		return (thrd_success);
	case ENOMEM:
		return (thrd_nomem);
	default:
		return (thrd_error);
	}
}

int
cnd_signal(cnd_t *cond)
{

	if (pthread_cond_signal(cond) != 0)
		return (thrd_error);
	return (thrd_success);
}

int
cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mtx,
    const struct timespec *restrict ts)
{

	switch (pthread_cond_timedwait(cond, mtx, ts)) {
	case 0:
		return (thrd_success);
	case ETIMEDOUT:
		return (thrd_timedout);
	default:
		return (thrd_error);
	}
}

int
cnd_wait(cnd_t *cond, mtx_t *mtx)
{

	if (pthread_cond_wait(cond, mtx) != 0)
		return (thrd_error);
	return (thrd_success);
}