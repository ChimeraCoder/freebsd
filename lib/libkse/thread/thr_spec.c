
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
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"


struct pthread_key _thread_keytable[PTHREAD_KEYS_MAX];

__weak_reference(_pthread_key_create, pthread_key_create);
__weak_reference(_pthread_key_delete, pthread_key_delete);
__weak_reference(_pthread_getspecific, pthread_getspecific);
__weak_reference(_pthread_setspecific, pthread_setspecific);


int
_pthread_key_create(pthread_key_t *key, void (*destructor) (void *))
{
	struct pthread *curthread;
	int i;

	if (_thr_initial == NULL)
		_libpthread_init(NULL);
	curthread = _get_curthread();

	/* Lock the key table: */
	THR_LOCK_ACQUIRE(curthread, &_keytable_lock);
	for (i = 0; i < PTHREAD_KEYS_MAX; i++) {

		if (_thread_keytable[i].allocated == 0) {
			_thread_keytable[i].allocated = 1;
			_thread_keytable[i].destructor = destructor;
			_thread_keytable[i].seqno++;

			/* Unlock the key table: */
			THR_LOCK_RELEASE(curthread, &_keytable_lock);
			*key = i;
			return (0);
		}

	}
	/* Unlock the key table: */
	THR_LOCK_RELEASE(curthread, &_keytable_lock);
	return (EAGAIN);
}

int
_pthread_key_delete(pthread_key_t key)
{
	struct pthread *curthread = _get_curthread();
	int ret = 0;

	if ((unsigned int)key < PTHREAD_KEYS_MAX) {
		/* Lock the key table: */
		THR_LOCK_ACQUIRE(curthread, &_keytable_lock);

		if (_thread_keytable[key].allocated)
			_thread_keytable[key].allocated = 0;
		else
			ret = EINVAL;

		/* Unlock the key table: */
		THR_LOCK_RELEASE(curthread, &_keytable_lock);
	} else
		ret = EINVAL;
	return (ret);
}

void 
_thread_cleanupspecific(void)
{
	struct pthread	*curthread = _get_curthread();
	const_key_destructor_t destructor;
	const void	*data = NULL;
	int		key;
	int		i;

	if (curthread->specific == NULL)
		return;

	/* Lock the key table: */
	THR_LOCK_ACQUIRE(curthread, &_keytable_lock);
	for (i = 0; (i < PTHREAD_DESTRUCTOR_ITERATIONS) &&
	    (curthread->specific_data_count > 0); i++) {
		for (key = 0; (key < PTHREAD_KEYS_MAX) &&
		    (curthread->specific_data_count > 0); key++) {
			destructor = NULL;

			if (_thread_keytable[key].allocated &&
			    (curthread->specific[key].data != NULL)) {
				if (curthread->specific[key].seqno ==
				    _thread_keytable[key].seqno) {
					data = curthread->specific[key].data;
					destructor = (const_key_destructor_t)
					    _thread_keytable[key].destructor;
				}
				curthread->specific[key].data = NULL;
				curthread->specific_data_count--;
			}

			/*
			 * If there is a destructore, call it
			 * with the key table entry unlocked:
			 */
			if (destructor != NULL) {
				/*
				 * Don't hold the lock while calling the
				 * destructor:
				 */
				THR_LOCK_RELEASE(curthread, &_keytable_lock);
				destructor(data);
				THR_LOCK_ACQUIRE(curthread, &_keytable_lock);
			}
		}
	}
	THR_LOCK_RELEASE(curthread, &_keytable_lock);
	free(curthread->specific);
	curthread->specific = NULL;
	if (curthread->specific_data_count > 0)
		stderr_debug("Thread %p has exited with leftover "
		    "thread-specific data after %d destructor iterations\n",
		    curthread, PTHREAD_DESTRUCTOR_ITERATIONS);
}

static inline struct pthread_specific_elem *
pthread_key_allocate_data(void)
{
	struct pthread_specific_elem *new_data;

	new_data = (struct pthread_specific_elem *)
	    malloc(sizeof(struct pthread_specific_elem) * PTHREAD_KEYS_MAX);
	if (new_data != NULL) {
		memset((void *) new_data, 0,
		    sizeof(struct pthread_specific_elem) * PTHREAD_KEYS_MAX);
	}
	return (new_data);
}

int 
_pthread_setspecific(pthread_key_t key, const void *value)
{
	struct pthread	*pthread;
	int		ret = 0;

	/* Point to the running thread: */
	pthread = _get_curthread();

	if ((pthread->specific) ||
	    (pthread->specific = pthread_key_allocate_data())) {
		if ((unsigned int)key < PTHREAD_KEYS_MAX) {
			if (_thread_keytable[key].allocated) {
				if (pthread->specific[key].data == NULL) {
					if (value != NULL)
						pthread->specific_data_count++;
				} else if (value == NULL)
					pthread->specific_data_count--;
				*(const void **)&pthread->specific[key].data = value;
				pthread->specific[key].seqno =
				    _thread_keytable[key].seqno;
				ret = 0;
			} else
				ret = EINVAL;
		} else
			ret = EINVAL;
	} else
		ret = ENOMEM;
	return (ret);
}

void *
_pthread_getspecific(pthread_key_t key)
{
	struct pthread	*pthread;
	void		*data;

	/* Point to the running thread: */
	pthread = _get_curthread();

	/* Check if there is specific data: */
	if (pthread->specific != NULL && (unsigned int)key < PTHREAD_KEYS_MAX) {
		/* Check if this key has been used before: */
		if (_thread_keytable[key].allocated &&
		    (pthread->specific[key].seqno == _thread_keytable[key].seqno)) {
			/* Return the value: */
			data = pthread->specific[key].data;
		} else {
			/*
			 * This key has not been used before, so return NULL
			 * instead: 
			 */
			data = NULL;
		}
	} else
		/* No specific data has been created, so just return NULL: */
		data = NULL;
	return (data);
}