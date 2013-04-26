
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
#include <pthread.h>
#include <stdio.h>

#define NLOOPS	10

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

int wake;
int stop;

void *
thr_routine(void *arg)
{
	pthread_mutex_lock(&m);
	while (wake == 0)
		pthread_cond_wait(&cv, &m);
	pthread_mutex_unlock(&m);

	while (stop == 0)
		pthread_yield();
	return (NULL);
}

int main(int argc, char **argv)
{
	pthread_t td;
	int i;
	void *result;

	pthread_setconcurrency(1);
	for (i = 0; i < NLOOPS; ++i) {
		stop = 0;
		wake = 0;

		pthread_create(&td, NULL, thr_routine, NULL);
		sleep(1);
		printf("trying: %d\n", i);
		pthread_mutex_lock(&m);
		wake = 1;
		pthread_cond_signal(&cv);
		pthread_cancel(td);
		pthread_mutex_unlock(&m);
		stop = 1;
		result = NULL;
		pthread_join(td, &result);
		if (result == PTHREAD_CANCELED) {
			printf("the condition variable implementation does not\n"
			       "conform to SUSv3, a thread unblocked from\n"
			       "condition variable still can be canceled.\n");
			return (1);
		}
	}

	printf("OK\n");
	return (0);
}