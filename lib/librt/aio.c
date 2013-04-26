
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
#include <sys/aio.h>

#include "namespace.h"
#include <errno.h>
#include <stddef.h>
#include <signal.h>
#include "sigev_thread.h"
#include "un-namespace.h"

__weak_reference(__aio_read, _aio_read);
__weak_reference(__aio_read, aio_read);
__weak_reference(__aio_write, _aio_write);
__weak_reference(__aio_write, aio_write);
__weak_reference(__aio_return, _aio_return);
__weak_reference(__aio_return, aio_return);
__weak_reference(__aio_waitcomplete, _aio_waitcomplete);
__weak_reference(__aio_waitcomplete, aio_waitcomplete);
__weak_reference(__aio_fsync, _aio_fsync);
__weak_reference(__aio_fsync, aio_fsync);

typedef void (*aio_func)(union sigval val, struct aiocb *iocb);

extern int __sys_aio_read(struct aiocb *iocb);
extern int __sys_aio_write(struct aiocb *iocb);
extern int __sys_aio_waitcomplete(struct aiocb **iocbp, struct timespec *timeout);
extern int __sys_aio_return(struct aiocb *iocb);
extern int __sys_aio_error(struct aiocb *iocb);
extern int __sys_aio_fsync(int op, struct aiocb *iocb);

static void
aio_dispatch(struct sigev_node *sn)
{
	aio_func f = sn->sn_func;

	f(sn->sn_value, (struct aiocb *)sn->sn_id);
}

static int
aio_sigev_alloc(struct aiocb *iocb, struct sigev_node **sn,
	struct sigevent *saved_ev)
{
	if (__sigev_check_init()) {
		/* This might be that thread library is not enabled. */
		errno = EINVAL;
		return (-1);
	}

	*sn = __sigev_alloc(SI_ASYNCIO, &iocb->aio_sigevent, NULL, 1);
	if (*sn == NULL) {
		errno = EAGAIN;
		return (-1);
	}
	
	*saved_ev = iocb->aio_sigevent;
	(*sn)->sn_id = (sigev_id_t)iocb;
	__sigev_get_sigevent(*sn, &iocb->aio_sigevent, (*sn)->sn_id);
	(*sn)->sn_dispatch = aio_dispatch;

	__sigev_list_lock();
	__sigev_register(*sn);
	__sigev_list_unlock();

	return (0);
}

static int
aio_io(struct aiocb *iocb, int (*sysfunc)(struct aiocb *iocb))
{
	struct sigev_node *sn;
	struct sigevent saved_ev;
	int ret, err;

	if (iocb->aio_sigevent.sigev_notify != SIGEV_THREAD) {
		ret = sysfunc(iocb);
		return (ret);
	}

	ret = aio_sigev_alloc(iocb, &sn, &saved_ev);
	if (ret)
		return (ret);
	ret = sysfunc(iocb);
	iocb->aio_sigevent = saved_ev;
	if (ret != 0) {
		err = errno;
		__sigev_list_lock();
		__sigev_delete_node(sn);
		__sigev_list_unlock();
		errno = err;
	}
	return (ret);
}

int
__aio_read(struct aiocb *iocb)
{

	return aio_io(iocb, &__sys_aio_read);
}

int
__aio_write(struct aiocb *iocb)
{

	return aio_io(iocb, &__sys_aio_write);
}

int
__aio_waitcomplete(struct aiocb **iocbp, struct timespec *timeout)
{
	int err;
	int ret = __sys_aio_waitcomplete(iocbp, timeout);

	if (*iocbp) {
		if ((*iocbp)->aio_sigevent.sigev_notify == SIGEV_THREAD) {
			err = errno;
			__sigev_list_lock();
			__sigev_delete(SI_ASYNCIO, (sigev_id_t)(*iocbp));
			__sigev_list_unlock();
			errno = err;
		}
	}

	return (ret);
}

int
__aio_return(struct aiocb *iocb)
{

	if (iocb->aio_sigevent.sigev_notify == SIGEV_THREAD) {
		if (__sys_aio_error(iocb) == EINPROGRESS)
			return (EINPROGRESS);
		__sigev_list_lock();
		__sigev_delete(SI_ASYNCIO, (sigev_id_t)iocb);
		__sigev_list_unlock();
	}

	return __sys_aio_return(iocb);
}

int
__aio_fsync(int op, struct aiocb *iocb)
{
	struct sigev_node *sn;
	struct sigevent saved_ev;
	int ret, err;

	if (iocb->aio_sigevent.sigev_notify != SIGEV_THREAD)
		return __sys_aio_fsync(op, iocb);

	ret = aio_sigev_alloc(iocb, &sn, &saved_ev);
	if (ret)
		return (ret);
	ret = __sys_aio_fsync(op, iocb);
	iocb->aio_sigevent = saved_ev;
	if (ret != 0) {
		err = errno;
		__sigev_list_lock();
		__sigev_delete_node(sn);
		__sigev_list_unlock();
		errno = err;
	}
	return (ret);
}