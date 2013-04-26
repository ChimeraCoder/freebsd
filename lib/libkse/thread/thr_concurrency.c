
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
#include <pthread.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include "un-namespace.h"

#include "thr_private.h"

/*#define DEBUG_CONCURRENCY */
#ifdef DEBUG_CONCURRENCY
#define DBG_MSG		stdout_debug
#else
#define	DBG_MSG(x...)
#endif

static int level = 0;

__weak_reference(_pthread_getconcurrency, pthread_getconcurrency);
__weak_reference(_pthread_setconcurrency, pthread_setconcurrency);

int
_pthread_getconcurrency(void)
{
	return (level);
}

int
_pthread_setconcurrency(int new_level)
{
	int ret;

	if (new_level < 0)
		ret = EINVAL;
	else if (new_level == level)
		ret = 0;
	else if (new_level == 0) {
		level = 0;
		ret = 0;
	} else if ((_kse_isthreaded() == 0) && (_kse_setthreaded(1) != 0)) {
		DBG_MSG("Can't enable threading.\n");
		ret = EAGAIN;
	} else {
		ret = _thr_setconcurrency(new_level);
		if (ret == 0)
			level = new_level;
	}
	return (ret);
}

int
_thr_setconcurrency(int new_level)
{
	struct pthread *curthread;
	struct kse *newkse, *kse;
	kse_critical_t crit;
	int kse_count;
	int i;
	int ret;

	/*
	 * Turn on threaded mode, if failed, it is unnecessary to
	 * do further work.
	 */
	if (_kse_isthreaded() == 0 && _kse_setthreaded(1))
		return (EAGAIN);

	ret = 0;
	curthread = _get_curthread();
	/* Race condition, but so what. */
	kse_count = _kse_initial->k_kseg->kg_ksecount;
	if (new_level > kse_count) {
		for (i = kse_count; i < new_level; i++) {
			newkse = _kse_alloc(curthread, 0);
			if (newkse == NULL) {
				DBG_MSG("Can't alloc new KSE.\n");
				ret = EAGAIN;
				break;
			}
			newkse->k_kseg = _kse_initial->k_kseg;
			newkse->k_schedq = _kse_initial->k_schedq;
			newkse->k_curthread = NULL;
			crit = _kse_critical_enter();
			KSE_SCHED_LOCK(curthread->kse, newkse->k_kseg);
			TAILQ_INSERT_TAIL(&newkse->k_kseg->kg_kseq,
			    newkse, k_kgqe);
			newkse->k_kseg->kg_ksecount++;
			newkse->k_flags |= KF_STARTED;
			KSE_SCHED_UNLOCK(curthread->kse, newkse->k_kseg);
			if (kse_create(&newkse->k_kcb->kcb_kmbx, 0) != 0) {
				KSE_SCHED_LOCK(curthread->kse, newkse->k_kseg);
				TAILQ_REMOVE(&newkse->k_kseg->kg_kseq,
				    newkse, k_kgqe);
				newkse->k_kseg->kg_ksecount--;
				KSE_SCHED_UNLOCK(curthread->kse,
				    newkse->k_kseg);
				_kse_critical_leave(crit);
				_kse_free(curthread, newkse);
				DBG_MSG("kse_create syscall failed.\n");
				ret = EAGAIN;
				break;
			} else {
				_kse_critical_leave(crit);
			}
		}
	} else if (new_level < kse_count) {
		kse_count = 0;
		crit = _kse_critical_enter();
		KSE_SCHED_LOCK(curthread->kse, _kse_initial->k_kseg);
		/* Count the number of active KSEs */
		TAILQ_FOREACH(kse, &_kse_initial->k_kseg->kg_kseq, k_kgqe) {
			if ((kse->k_flags & KF_TERMINATED) == 0)
				kse_count++;
		}
		/* Reduce the number of active KSEs appropriately. */
		kse = TAILQ_FIRST(&_kse_initial->k_kseg->kg_kseq);
		while ((kse != NULL) && (kse_count > new_level)) {
			if ((kse != _kse_initial) &&
			    ((kse->k_flags & KF_TERMINATED) == 0)) {
				kse->k_flags |= KF_TERMINATED;
				kse_count--;
				/* Wakup the KSE in case it is idle. */
				kse_wakeup(&kse->k_kcb->kcb_kmbx);
			}
			kse = TAILQ_NEXT(kse, k_kgqe);
		}
		KSE_SCHED_UNLOCK(curthread->kse, _kse_initial->k_kseg);
		_kse_critical_leave(crit);
	}
	return (ret);
}

int
_thr_setmaxconcurrency(void)
{
	int vcpu;
	size_t len;
	int ret;

	len = sizeof(vcpu);
	ret = sysctlbyname("kern.threads.virtual_cpu", &vcpu, &len, NULL, 0);
	if (ret == 0 && vcpu > 0)
		ret = _thr_setconcurrency(vcpu);
	return (ret);
}