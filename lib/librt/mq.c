
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
#include <sys/mqueue.h>

#include "namespace.h"
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <signal.h>
#include "sigev_thread.h"
#include "un-namespace.h"
#include "libc_private.h"

extern int	__sys_kmq_notify(int, const struct sigevent *);
extern int	__sys_kmq_open(const char *, int, mode_t,
		    const struct mq_attr *);
extern int	__sys_kmq_setattr(int, const struct mq_attr *__restrict,
		    struct mq_attr *__restrict);
extern ssize_t	__sys_kmq_timedreceive(int, char *__restrict, size_t,
		    unsigned *__restrict, const struct timespec *__restrict);
extern int	__sys_kmq_timedsend(int, const char *, size_t, unsigned,
		    const struct timespec *);
extern int	__sys_kmq_unlink(const char *);
extern int	__sys_close(int fd);

struct __mq {
	int oshandle;
	struct sigev_node *node;
};

__weak_reference(__mq_open, mq_open);
__weak_reference(__mq_open, _mq_open);
__weak_reference(__mq_close, mq_close);
__weak_reference(__mq_close, _mq_close);
__weak_reference(__mq_notify, mq_notify);
__weak_reference(__mq_notify, _mq_notify);
__weak_reference(__mq_getattr, mq_getattr);
__weak_reference(__mq_getattr, _mq_getattr);
__weak_reference(__mq_setattr, mq_setattr);
__weak_reference(__mq_setattr, _mq_setattr);
__weak_reference(__mq_timedreceive_cancel, mq_timedreceive);
__weak_reference(__mq_timedreceive, _mq_timedreceive);
__weak_reference(__mq_timedsend_cancel, mq_timedsend);
__weak_reference(__mq_timedsend, _mq_timedsend);
__weak_reference(__mq_unlink, mq_unlink);
__weak_reference(__mq_unlink, _mq_unlink);
__weak_reference(__mq_send_cancel, mq_send);
__weak_reference(__mq_send, _mq_send);
__weak_reference(__mq_receive_cancel, mq_receive);
__weak_reference(__mq_receive, _mq_receive);

mqd_t
__mq_open(const char *name, int oflag, mode_t mode,
	const struct mq_attr *attr)
{
	struct __mq *mq;
	int err;

	mq = malloc(sizeof(struct __mq));
	if (mq == NULL)
		return (NULL);

	mq->oshandle = __sys_kmq_open(name, oflag, mode, attr);
	if (mq->oshandle != -1) {
		mq->node = NULL;
		return (mq);
	}
	err = errno;
	free(mq);
	errno = err;
	return ((mqd_t)-1L);
}

int
__mq_close(mqd_t mqd)
{
	int h;

	if (mqd->node != NULL) {
		__sigev_list_lock();
		__sigev_delete_node(mqd->node);
		__sigev_list_unlock();
	}
	h = mqd->oshandle;
	free(mqd);
	return (__sys_close(h));
}

typedef void (*mq_func)(union sigval val);

static void
mq_dispatch(struct sigev_node *sn)
{
	mq_func f = sn->sn_func;

	/*
	 * Check generation before calling user function,
	 * this should avoid expired notification.
	 */
	if (sn->sn_gen == sn->sn_info.si_value.sival_int)
		f(sn->sn_value);
}

int
__mq_notify(mqd_t mqd, const struct sigevent *evp)
{
	struct sigevent ev;
	struct sigev_node *sn;
	int ret;

	if (evp == NULL || evp->sigev_notify != SIGEV_THREAD) {
		if (mqd->node != NULL) {
			__sigev_list_lock();
			__sigev_delete_node(mqd->node);
			mqd->node = NULL;
			__sigev_list_unlock();
		}
		return __sys_kmq_notify(mqd->oshandle, evp);
	}

	if (__sigev_check_init()) {
		/*
		 * Thread library is not enabled.
		 */
		errno = EINVAL;
		return (-1);
	}

	sn = __sigev_alloc(SI_MESGQ, evp, mqd->node, 1);
	if (sn == NULL) {
		errno = EAGAIN;
		return (-1);
	}

	sn->sn_id = mqd->oshandle;
	sn->sn_dispatch = mq_dispatch;
	__sigev_get_sigevent(sn, &ev, sn->sn_gen);
	__sigev_list_lock();
	if (mqd->node != NULL)
		__sigev_delete_node(mqd->node);
	mqd->node = sn;
	__sigev_register(sn);
	ret = __sys_kmq_notify(mqd->oshandle, &ev);
	__sigev_list_unlock();
	return (ret);
}

int
__mq_getattr(mqd_t mqd, struct mq_attr *attr)
{

	return __sys_kmq_setattr(mqd->oshandle, NULL, attr);
}

int
__mq_setattr(mqd_t mqd, const struct mq_attr *newattr, struct mq_attr *oldattr)
{

	return __sys_kmq_setattr(mqd->oshandle, newattr, oldattr);
}

ssize_t
__mq_timedreceive(mqd_t mqd, char *buf, size_t len,
	unsigned *prio, const struct timespec *timeout)
{

	return __sys_kmq_timedreceive(mqd->oshandle, buf, len, prio, timeout);
}

ssize_t
__mq_timedreceive_cancel(mqd_t mqd, char *buf, size_t len,
	unsigned *prio, const struct timespec *timeout)
{
	int ret;

	_pthread_cancel_enter(1);
	ret = __sys_kmq_timedreceive(mqd->oshandle, buf, len, prio, timeout);
	_pthread_cancel_leave(ret == -1);
	return (ret);
}

ssize_t
__mq_receive(mqd_t mqd, char *buf, size_t len, unsigned *prio)
{

	return __sys_kmq_timedreceive(mqd->oshandle, buf, len, prio, NULL);
}

ssize_t
__mq_receive_cancel(mqd_t mqd, char *buf, size_t len, unsigned *prio)
{
	int ret;

	_pthread_cancel_enter(1);
	ret = __sys_kmq_timedreceive(mqd->oshandle, buf, len, prio, NULL);
	_pthread_cancel_leave(ret == -1);
	return (ret);
}
ssize_t
__mq_timedsend(mqd_t mqd, char *buf, size_t len,
	unsigned prio, const struct timespec *timeout)
{

	return __sys_kmq_timedsend(mqd->oshandle, buf, len, prio, timeout);
}

ssize_t
__mq_timedsend_cancel(mqd_t mqd, char *buf, size_t len,
	unsigned prio, const struct timespec *timeout)
{
	int ret;

	_pthread_cancel_enter(1);
	ret = __sys_kmq_timedsend(mqd->oshandle, buf, len, prio, timeout);
	_pthread_cancel_leave(ret == -1);
	return (ret);
}

ssize_t
__mq_send(mqd_t mqd, char *buf, size_t len, unsigned prio)
{

	return __sys_kmq_timedsend(mqd->oshandle, buf, len, prio, NULL);
}


ssize_t
__mq_send_cancel(mqd_t mqd, char *buf, size_t len, unsigned prio)
{
	int ret;

	_pthread_cancel_enter(1);
	ret = __sys_kmq_timedsend(mqd->oshandle, buf, len, prio, NULL);
	_pthread_cancel_leave(ret == -1);
	return (ret);
}

int
__mq_unlink(const char *path)
{

	return __sys_kmq_unlink(path);
}

int
__mq_oshandle(mqd_t mqd)
{

	return (mqd->oshandle);
}