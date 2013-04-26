
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
/*
 * $FreeBSD$
 *
 * Test stack unwinding for mixed pthread_cleanup_push/pop and C++
 * object, both should work together.
 *
 */
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

#include "Test.cpp"

pthread_mutex_t mtx;
pthread_cond_t cv;

void f()
{
	Test t;

	pthread_mutex_lock(&mtx);
	pthread_cond_wait(&cv, &mtx);
	pthread_mutex_unlock(&mtx);
	printf("Bug, thread shouldn't be here.\n");
}

void g()
{
	f();
}

void *
thr(void *arg)
{
	pthread_cleanup_push(cleanup_handler, NULL);
	g();
	pthread_cleanup_pop(0);
	return (0);
}

int
main()
{
	pthread_t td;

	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&cv, NULL);
	pthread_create(&td, NULL, thr, NULL);
	sleep(1);
	pthread_cancel(td);
	pthread_join(td, NULL);
	check_destruct2();
	return (0);
}