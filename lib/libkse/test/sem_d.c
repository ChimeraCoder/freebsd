
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

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

#define NTHREADS 10

void *
entry(void * a_arg)
{
	sem_t * sem = (sem_t *) a_arg;

	sem_wait(sem);
	fprintf(stderr, "Got semaphore\n");
  
	return NULL;
}

int
main()
{
	sem_t sem_a, sem_b;
	pthread_t threads[NTHREADS];
	unsigned i;
	int val;
  
	fprintf(stderr, "Test begin\n");

#ifdef _LIBC_R_
	assert(-1 == sem_init(&sem_b, 1, 0));
	assert(EPERM == errno);
#endif

	assert(0 == sem_init(&sem_b, 0, 0));
	assert(0 == sem_getvalue(&sem_b, &val));
	assert(0 == val);
  
	assert(0 == sem_post(&sem_b));
	assert(0 == sem_getvalue(&sem_b, &val));
	assert(1 == val);
  
	assert(0 == sem_wait(&sem_b));
	assert(-1 == sem_trywait(&sem_b));
	assert(EAGAIN == errno);
	assert(0 == sem_post(&sem_b));
	assert(0 == sem_trywait(&sem_b));
	assert(0 == sem_post(&sem_b));
	assert(0 == sem_wait(&sem_b));
	assert(0 == sem_post(&sem_b));

#ifdef _LIBC_R_
	assert(SEM_FAILED == sem_open("/foo", O_CREAT | O_EXCL, 0644, 0));
	assert(ENOSYS == errno);

	assert(-1 == sem_close(&sem_b));
	assert(ENOSYS == errno);
  
	assert(-1 == sem_unlink("/foo"));
	assert(ENOSYS == errno);
#endif

	assert(0 == sem_destroy(&sem_b));
  
	assert(0 == sem_init(&sem_a, 0, 0));

	for (i = 0; i < NTHREADS; i++) {
		pthread_create(&threads[i], NULL, entry, (void *) &sem_a);
	}

	for (i = 0; i < NTHREADS; i++) {
		assert(0 == sem_post(&sem_a));
	}
  
	for (i = 0; i < NTHREADS; i++) {
		pthread_join(threads[i], NULL);
	}
  
	for (i = 0; i < NTHREADS; i++) {
		pthread_create(&threads[i], NULL, entry, (void *) &sem_a);
	}

	for (i = 0; i < NTHREADS; i++) {
		assert(0 == sem_post(&sem_a));
	}
  
	for (i = 0; i < NTHREADS; i++) {
		pthread_join(threads[i], NULL);
	}
  
	assert(0 == sem_destroy(&sem_a));

	fprintf(stderr, "Test end\n");
	return 0;
}