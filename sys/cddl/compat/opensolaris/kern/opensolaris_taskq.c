
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/kmem.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/queue.h>
#include <sys/taskqueue.h>
#include <sys/taskq.h>

#include <vm/uma.h>

static uma_zone_t taskq_zone;

taskq_t *system_taskq = NULL;

static void
system_taskq_init(void *arg)
{

	taskq_zone = uma_zcreate("taskq_zone", sizeof(struct ostask),
	    NULL, NULL, NULL, NULL, 0, 0);
	system_taskq = taskq_create("system_taskq", mp_ncpus, 0, 0, 0, 0);
}
SYSINIT(system_taskq_init, SI_SUB_CONFIGURE, SI_ORDER_ANY, system_taskq_init, NULL);

static void
system_taskq_fini(void *arg)
{

	taskq_destroy(system_taskq);
	uma_zdestroy(taskq_zone);
}
SYSUNINIT(system_taskq_fini, SI_SUB_CONFIGURE, SI_ORDER_ANY, system_taskq_fini, NULL);

taskq_t *
taskq_create(const char *name, int nthreads, pri_t pri, int minalloc __unused,
    int maxalloc __unused, uint_t flags)
{
	taskq_t *tq;

	if ((flags & TASKQ_THREADS_CPU_PCT) != 0)
		nthreads = MAX((mp_ncpus * nthreads) / 100, 1);

	tq = kmem_alloc(sizeof(*tq), KM_SLEEP);
	tq->tq_queue = taskqueue_create(name, M_WAITOK, taskqueue_thread_enqueue,
	    &tq->tq_queue);
	(void) taskqueue_start_threads(&tq->tq_queue, nthreads, pri, "%s", name);

	return ((taskq_t *)tq);
}

taskq_t *
taskq_create_proc(const char *name, int nthreads, pri_t pri, int minalloc,
    int maxalloc, proc_t *proc __unused, uint_t flags)
{

	return (taskq_create(name, nthreads, pri, minalloc, maxalloc, flags));
}

void
taskq_destroy(taskq_t *tq)
{

	taskqueue_free(tq->tq_queue);
	kmem_free(tq, sizeof(*tq));
}

int
taskq_member(taskq_t *tq, kthread_t *thread)
{

	return (taskqueue_member(tq->tq_queue, thread));
}

static void
taskq_run(void *arg, int pending __unused)
{
	struct ostask *task = arg;

	task->ost_func(task->ost_arg);

	uma_zfree(taskq_zone, task);
}

taskqid_t
taskq_dispatch(taskq_t *tq, task_func_t func, void *arg, uint_t flags)
{
	struct ostask *task;
	int mflag, prio;

	if ((flags & (TQ_SLEEP | TQ_NOQUEUE)) == TQ_SLEEP)
		mflag = M_WAITOK;
	else
		mflag = M_NOWAIT;
	/* 
	 * If TQ_FRONT is given, we want higher priority for this task, so it
	 * can go at the front of the queue.
	 */
	prio = !!(flags & TQ_FRONT);

	task = uma_zalloc(taskq_zone, mflag);
	if (task == NULL)
		return (0);

	task->ost_func = func;
	task->ost_arg = arg;

	TASK_INIT(&task->ost_task, prio, taskq_run, task);
	taskqueue_enqueue(tq->tq_queue, &task->ost_task);

	return ((taskqid_t)(void *)task);
}

#define	TASKQ_MAGIC	0x74541c

static void
taskq_run_safe(void *arg, int pending __unused)
{
	struct ostask *task = arg;

	task->ost_func(task->ost_arg);
}

taskqid_t
taskq_dispatch_safe(taskq_t *tq, task_func_t func, void *arg, u_int flags,
    struct ostask *task)
{
	int prio;

	/* 
	 * If TQ_FRONT is given, we want higher priority for this task, so it
	 * can go at the front of the queue.
	 */
	prio = !!(flags & TQ_FRONT);

	task->ost_func = func;
	task->ost_arg = arg;

	TASK_INIT(&task->ost_task, prio, taskq_run_safe, task);
	taskqueue_enqueue(tq->tq_queue, &task->ost_task);

	return ((taskqid_t)(void *)task);
}