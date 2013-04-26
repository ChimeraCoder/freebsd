
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

#include <sys/types.h>
#include <sys/signalvar.h>
#include <sys/rtprio.h>
#include <sys/mman.h>
#include <pthread.h>

#include "thr_private.h"

/*#define DEBUG_THREAD_KERN */
#ifdef DEBUG_THREAD_KERN
#define DBG_MSG		stdout_debug
#else
#define DBG_MSG(x...)
#endif

static struct umutex	addr_lock;
static struct wake_addr *wake_addr_head;
static struct wake_addr default_wake_addr;

/*
 * This is called when the first thread (other than the initial
 * thread) is created.
 */
int
_thr_setthreaded(int threaded)
{
	if (((threaded == 0) ^ (__isthreaded == 0)) == 0)
		return (0);

	__isthreaded = threaded;
	return (0);
}

void
_thr_assert_lock_level()
{
	PANIC("locklevel <= 0");
}

int
_rtp_to_schedparam(const struct rtprio *rtp, int *policy,
	struct sched_param *param)
{
	switch(rtp->type) {
	case RTP_PRIO_REALTIME:
		*policy = SCHED_RR;
		param->sched_priority = RTP_PRIO_MAX - rtp->prio;
		break;
	case RTP_PRIO_FIFO:
		*policy = SCHED_FIFO;
		param->sched_priority = RTP_PRIO_MAX - rtp->prio;
		break;
	default:
		*policy = SCHED_OTHER;
		param->sched_priority = 0;
		break;
	}
	return (0);
}

int
_schedparam_to_rtp(int policy, const struct sched_param *param,
	struct rtprio *rtp)
{
	switch(policy) {
	case SCHED_RR:
		rtp->type = RTP_PRIO_REALTIME;
		rtp->prio = RTP_PRIO_MAX - param->sched_priority;
		break;
	case SCHED_FIFO:
		rtp->type = RTP_PRIO_FIFO;
		rtp->prio = RTP_PRIO_MAX - param->sched_priority;
		break;
	case SCHED_OTHER:
	default:
		rtp->type = RTP_PRIO_NORMAL;
		rtp->prio = 0;
		break;
	}
	return (0);
}

int
_thr_getscheduler(lwpid_t lwpid, int *policy, struct sched_param *param)
{
	struct rtprio rtp;
	int ret;

	ret = rtprio_thread(RTP_LOOKUP, lwpid, &rtp);
	if (ret == -1)
		return (ret);
	_rtp_to_schedparam(&rtp, policy, param);
	return (0);
}

int
_thr_setscheduler(lwpid_t lwpid, int policy, const struct sched_param *param)
{
	struct rtprio rtp;

	_schedparam_to_rtp(policy, param, &rtp);
	return (rtprio_thread(RTP_SET, lwpid, &rtp));
}

void
_thr_wake_addr_init(void)
{
	_thr_umutex_init(&addr_lock);
	wake_addr_head = NULL;
}

/*
 * Allocate wake-address, the memory area is never freed after
 * allocated, this becauses threads may be referencing it.
 */
struct wake_addr *
_thr_alloc_wake_addr(void)
{
	struct pthread *curthread;
	struct wake_addr *p;

	if (_thr_initial == NULL) {
		return &default_wake_addr;
	}

	curthread = _get_curthread();

	THR_LOCK_ACQUIRE(curthread, &addr_lock);
	if (wake_addr_head == NULL) {
		unsigned i;
		unsigned pagesize = getpagesize();
		struct wake_addr *pp = (struct wake_addr *)
			mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE,
			MAP_ANON|MAP_PRIVATE, -1, 0);
		for (i = 1; i < pagesize/sizeof(struct wake_addr); ++i)
			pp[i].link = &pp[i+1];
		pp[i-1].link = NULL;	
		wake_addr_head = &pp[1];
		p = &pp[0];
	} else {
		p = wake_addr_head;
		wake_addr_head = p->link;
	}
	THR_LOCK_RELEASE(curthread, &addr_lock);
	p->value = 0;
	return (p);
}

void
_thr_release_wake_addr(struct wake_addr *wa)
{
	struct pthread *curthread = _get_curthread();

	if (wa == &default_wake_addr)
		return;
	THR_LOCK_ACQUIRE(curthread, &addr_lock);
	wa->link = wake_addr_head;
	wake_addr_head = wa;
	THR_LOCK_RELEASE(curthread, &addr_lock);
}

/* Sleep on thread wakeup address */
int
_thr_sleep(struct pthread *curthread, int clockid,
	const struct timespec *abstime)
{

	if (curthread->wake_addr->value != 0)
		return (0);

	return _thr_umtx_timedwait_uint(&curthread->wake_addr->value, 0,
                 clockid, abstime, 0);
}

void
_thr_wake_all(unsigned int *waddrs[], int count)
{
	int i;

	for (i = 0; i < count; ++i)
		*waddrs[i] = 1;
	_umtx_op(waddrs, UMTX_OP_NWAKE_PRIVATE, count, NULL, NULL);
}