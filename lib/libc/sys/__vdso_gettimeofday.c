
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

#include <sys/elf.h>
#include <sys/time.h>
#include <sys/vdso.h>
#include <errno.h>
#include <time.h>
#include <machine/atomic.h>
#include "libc_private.h"

static u_int
tc_delta(const struct vdso_timehands *th)
{

	return ((__vdso_gettc(th) - th->th_offset_count) &
	    th->th_counter_mask);
}

static int
binuptime(struct bintime *bt, struct vdso_timekeep *tk, int abs)
{
	struct vdso_timehands *th;
	uint32_t curr, gen;

	do {
		if (!tk->tk_enabled)
			return (ENOSYS);

		/*
		 * XXXKIB. The load of tk->tk_current should use
		 * atomic_load_acq_32 to provide load barrier. But
		 * since tk points to r/o mapped page, x86
		 * implementation of atomic_load_acq faults.
		 */
		curr = tk->tk_current;
		rmb();
		th = &tk->tk_th[curr];
		if (th->th_algo != VDSO_TH_ALGO_1)
			return (ENOSYS);
		gen = th->th_gen;
		*bt = th->th_offset;
		bintime_addx(bt, th->th_scale * tc_delta(th));
		if (abs)
			bintime_add(bt, &th->th_boottime);

		/*
		 * Barrier for load of both tk->tk_current and th->th_gen.
		 */
		rmb();
	} while (curr != tk->tk_current || gen == 0 || gen != th->th_gen);
	return (0);
}

static struct vdso_timekeep *tk;

#pragma weak __vdso_gettimeofday
int
__vdso_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct bintime bt;
	int error;

	if (tz != NULL)
		return (ENOSYS);
	if (tk == NULL) {
		error = __vdso_gettimekeep(&tk);
		if (error != 0 || tk == NULL)
			return (ENOSYS);
	}
	if (tk->tk_ver != VDSO_TK_VER_CURR)
		return (ENOSYS);
	error = binuptime(&bt, tk, 1);
	if (error != 0)
		return (error);
	bintime2timeval(&bt, tv);
	return (0);
}

#pragma weak __vdso_clock_gettime
int
__vdso_clock_gettime(clockid_t clock_id, struct timespec *ts)
{
	struct bintime bt;
	int abs, error;

	if (tk == NULL) {
		error = _elf_aux_info(AT_TIMEKEEP, &tk, sizeof(tk));
		if (error != 0 || tk == NULL)
			return (ENOSYS);
	}
	if (tk->tk_ver != VDSO_TK_VER_CURR)
		return (ENOSYS);
	switch (clock_id) {
	case CLOCK_REALTIME:
	case CLOCK_REALTIME_PRECISE:
	case CLOCK_REALTIME_FAST:
	case CLOCK_SECOND:
		abs = 1;
		break;
	case CLOCK_MONOTONIC:
	case CLOCK_MONOTONIC_PRECISE:
	case CLOCK_MONOTONIC_FAST:
	case CLOCK_UPTIME:
	case CLOCK_UPTIME_PRECISE:
	case CLOCK_UPTIME_FAST:
		abs = 0;
		break;
	default:
		return (ENOSYS);
	}
	error = binuptime(&bt, tk, abs);
	if (error != 0)
		return (error);
	bintime2timespec(&bt, ts);
	if (clock_id == CLOCK_SECOND)
		ts->tv_nsec = 0;
	return (0);
}