
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
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/queue.h>
#include <sys/sysctl.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/posix4.h>

static int facility[CTL_P1003_1B_MAXID - 1];
static int facility_initialized[CTL_P1003_1B_MAXID - 1];

static int p31b_sysctl_proc(SYSCTL_HANDLER_ARGS);

/* OID_AUTO isn't working with sysconf(3).  I guess I'd have to
 * modify it to do a lookup by name from the index.
 * For now I've left it a top-level sysctl.
 */

#if 1

SYSCTL_DECL(_p1003_1b);

#define P1B_SYSCTL(num, name)  \
	SYSCTL_INT(_p1003_1b, num, name, CTLFLAG_RD | CTLFLAG_CAPRD, \
	facility + num - 1, 0, "");
#define P1B_SYSCTL_RW(num, name)  \
	SYSCTL_PROC(_p1003_1b, num, name, CTLTYPE_INT | CTLFLAG_RW, NULL, num, \
	    p31b_sysctl_proc, "I", "");

#else

SYSCTL_DECL(_kern_p1003_1b);

#define P1B_SYSCTL(num, name)  \
	SYSCTL_INT(_kern_p1003_1b, OID_AUTO, name, CTLFLAG_RD | CTLFLAG_CAPRD, \
	    facility + num - 1, 0, "");
#define P1B_SYSCTL_RW(num, name)  \
	SYSCTL_PROC(_p1003_1b, OID_AUTO, name, CTLTYPE_INT | CTLFLAG_RW, NULL, \
	    num, p31b_sysctl_proc, "I", "");
SYSCTL_NODE(_kern, OID_AUTO, p1003_1b, CTLFLAG_RW, 0, "P1003.1B");

#endif

SYSCTL_INT(_p1003_1b, CTL_P1003_1B_ASYNCHRONOUS_IO, \
	asynchronous_io, CTLFLAG_RD, &async_io_version, 0, "");
P1B_SYSCTL(CTL_P1003_1B_MAPPED_FILES, mapped_files);
P1B_SYSCTL(CTL_P1003_1B_MEMLOCK, memlock);
P1B_SYSCTL(CTL_P1003_1B_MEMLOCK_RANGE, memlock_range);
P1B_SYSCTL(CTL_P1003_1B_MEMORY_PROTECTION, memory_protection);
P1B_SYSCTL(CTL_P1003_1B_MESSAGE_PASSING, message_passing);
P1B_SYSCTL(CTL_P1003_1B_PRIORITIZED_IO, prioritized_io);
P1B_SYSCTL(CTL_P1003_1B_PRIORITY_SCHEDULING, priority_scheduling);
P1B_SYSCTL(CTL_P1003_1B_REALTIME_SIGNALS, realtime_signals);
P1B_SYSCTL(CTL_P1003_1B_SEMAPHORES, semaphores);
P1B_SYSCTL(CTL_P1003_1B_FSYNC, fsync);
P1B_SYSCTL(CTL_P1003_1B_SHARED_MEMORY_OBJECTS, shared_memory_objects);
P1B_SYSCTL(CTL_P1003_1B_SYNCHRONIZED_IO, synchronized_io);
P1B_SYSCTL(CTL_P1003_1B_TIMERS, timers);
P1B_SYSCTL(CTL_P1003_1B_AIO_LISTIO_MAX, aio_listio_max);
P1B_SYSCTL(CTL_P1003_1B_AIO_MAX, aio_max);
P1B_SYSCTL(CTL_P1003_1B_AIO_PRIO_DELTA_MAX, aio_prio_delta_max);
P1B_SYSCTL(CTL_P1003_1B_DELAYTIMER_MAX, delaytimer_max);
P1B_SYSCTL(CTL_P1003_1B_MQ_OPEN_MAX, mq_open_max);
P1B_SYSCTL(CTL_P1003_1B_PAGESIZE, pagesize);
P1B_SYSCTL(CTL_P1003_1B_RTSIG_MAX, rtsig_max);
P1B_SYSCTL_RW(CTL_P1003_1B_SEM_NSEMS_MAX, sem_nsems_max);
P1B_SYSCTL(CTL_P1003_1B_SEM_VALUE_MAX, sem_value_max);
P1B_SYSCTL(CTL_P1003_1B_SIGQUEUE_MAX, sigqueue_max);
P1B_SYSCTL(CTL_P1003_1B_TIMER_MAX, timer_max);

#define P31B_VALID(num)	((num) >= 1 && (num) < CTL_P1003_1B_MAXID)

static int
p31b_sysctl_proc(SYSCTL_HANDLER_ARGS)
{
	int error, num, val;

	num = arg2;
	if (!P31B_VALID(num))
		return (EINVAL);
	val = facility_initialized[num] ? facility[num - 1] : 0;
	error = sysctl_handle_int(oidp, &val, 0, req);
	if (error == 0 && req->newptr != NULL && facility_initialized[num])
		facility[num - 1] = val;
	return (error);
}

/* p31b_setcfg: Set the configuration
 */
void
p31b_setcfg(int num, int value)
{

	if (P31B_VALID(num)) {
		facility[num - 1] = value;
		facility_initialized[num - 1] = 1;
	}
}

void
p31b_unsetcfg(int num)
{

	facility[num - 1] = 0;
	facility_initialized[num -1] = 0;
}

int
p31b_getcfg(int num)
{

	if (P31B_VALID(num))
		return (facility[num - 1]);
	return (0);
}

int
p31b_iscfg(int num)
{

	if (P31B_VALID(num))
		return (facility_initialized[num - 1]);
	return (0);
}

/*
 * Turn on indications for standard (non-configurable) kernel features.
 */
static void
p31b_set_standard(void *dummy)
{

	p31b_setcfg(CTL_P1003_1B_FSYNC, 200112L);
	p31b_setcfg(CTL_P1003_1B_MAPPED_FILES, 200112L);
	p31b_setcfg(CTL_P1003_1B_SHARED_MEMORY_OBJECTS, 200112L);
	p31b_setcfg(CTL_P1003_1B_PAGESIZE, PAGE_SIZE);
	if (!p31b_iscfg(CTL_P1003_1B_AIO_LISTIO_MAX))
		p31b_setcfg(CTL_P1003_1B_AIO_LISTIO_MAX, -1);
	if (!p31b_iscfg(CTL_P1003_1B_AIO_MAX))
		p31b_setcfg(CTL_P1003_1B_AIO_MAX, -1);
	if (!p31b_iscfg(CTL_P1003_1B_AIO_PRIO_DELTA_MAX))
		p31b_setcfg(CTL_P1003_1B_AIO_PRIO_DELTA_MAX, -1);
}

SYSINIT(p31b_set_standard, SI_SUB_P1003_1B, SI_ORDER_ANY, p31b_set_standard, 
	0);