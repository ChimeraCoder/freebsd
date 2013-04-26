
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

#include "hi_locl.h"
#ifdef HAVE_GCD
#include <dispatch/dispatch.h>
#else
#include "heim_threads.h"
#endif

struct heim_icred {
    uid_t uid;
    gid_t gid;
    pid_t pid;
    pid_t session;
};

void
heim_ipc_free_cred(heim_icred cred)
{
    free(cred);
}

uid_t
heim_ipc_cred_get_uid(heim_icred cred)
{
    return cred->uid;
}

gid_t
heim_ipc_cred_get_gid(heim_icred cred)
{
    return cred->gid;
}

pid_t
heim_ipc_cred_get_pid(heim_icred cred)
{
    return cred->pid;
}

pid_t
heim_ipc_cred_get_session(heim_icred cred)
{
    return cred->session;
}


int
_heim_ipc_create_cred(uid_t uid, gid_t gid, pid_t pid, pid_t session, heim_icred *cred)
{
    *cred = calloc(1, sizeof(**cred));
    if (*cred == NULL)
	return ENOMEM;
    (*cred)->uid = uid;
    (*cred)->gid = gid;
    (*cred)->pid = pid;
    (*cred)->session = session;
    return 0;
}

#ifndef HAVE_GCD
struct heim_isemaphore {
    HEIMDAL_MUTEX mutex;
    pthread_cond_t cond;
    long counter;
};
#endif

heim_isemaphore
heim_ipc_semaphore_create(long value)
{
#ifdef HAVE_GCD
    return (heim_isemaphore)dispatch_semaphore_create(value);
#elif !defined(ENABLE_PTHREAD_SUPPORT)
    heim_assert(0, "no semaphore support w/o pthreads");
    return NULL;
#else
    heim_isemaphore s = malloc(sizeof(*s));
    if (s == NULL)
	return NULL;
    HEIMDAL_MUTEX_init(&s->mutex);
    pthread_cond_init(&s->cond, NULL);
    s->counter = value;
    return s;
#endif
}

long
heim_ipc_semaphore_wait(heim_isemaphore s, time_t t)
{
#ifdef HAVE_GCD
    uint64_t timeout;
    if (t == HEIM_IPC_WAIT_FOREVER)
	timeout = DISPATCH_TIME_FOREVER;
    else
	timeout = (uint64_t)t * NSEC_PER_SEC;

    return dispatch_semaphore_wait((dispatch_semaphore_t)s, timeout);
#elif !defined(ENABLE_PTHREAD_SUPPORT)
    heim_assert(0, "no semaphore support w/o pthreads");
    return 0;
#else
    HEIMDAL_MUTEX_lock(&s->mutex);
    /* if counter hits below zero, we get to wait */
    if (--s->counter < 0) {
	int ret;

	if (t == HEIM_IPC_WAIT_FOREVER)
	    ret = pthread_cond_wait(&s->cond, &s->mutex);
	else {
	    struct timespec ts;
	    ts.tv_sec = t;
	    ts.tv_nsec = 0;
	    ret = pthread_cond_timedwait(&s->cond, &s->mutex, &ts);
	}
	if (ret) {
	    HEIMDAL_MUTEX_unlock(&s->mutex);
	    return errno;
	}
    }
    HEIMDAL_MUTEX_unlock(&s->mutex);

    return 0;
#endif
}

long
heim_ipc_semaphore_signal(heim_isemaphore s)
{
#ifdef HAVE_GCD
    return dispatch_semaphore_signal((dispatch_semaphore_t)s);
#elif !defined(ENABLE_PTHREAD_SUPPORT)
    heim_assert(0, "no semaphore support w/o pthreads");
    return EINVAL;
#else
    int wakeup;
    HEIMDAL_MUTEX_lock(&s->mutex);
    wakeup = (++s->counter == 0) ;
    HEIMDAL_MUTEX_unlock(&s->mutex);
    if (wakeup)
	pthread_cond_signal(&s->cond);
    return 0;
#endif
}

void
heim_ipc_semaphore_release(heim_isemaphore s)
{
#ifdef HAVE_GCD
    dispatch_release((dispatch_semaphore_t)s);
#elif !defined(ENABLE_PTHREAD_SUPPORT)
    heim_assert(0, "no semaphore support w/o pthreads");
#else
    HEIMDAL_MUTEX_lock(&s->mutex);
    if (s->counter != 0)
	abort();
    HEIMDAL_MUTEX_unlock(&s->mutex);
    HEIMDAL_MUTEX_destroy(&s->mutex);
    pthread_cond_destroy(&s->cond);
    free(s);
#endif
}

void
heim_ipc_free_data(heim_idata *data)
{
    if (data->data)
	free(data->data);
    data->data = NULL;
    data->length = 0;
}