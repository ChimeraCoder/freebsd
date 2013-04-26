
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
/* $FreeBSD$ */
#include <stdio.h>
#include <pthread.h>

int __thread i;

void *
foo1(void *arg)
{
	printf("thread %p, &i = %p\n", pthread_self(), &i);
	for (i = 0; i < 10; i++) {
		printf("thread %p, i = %d\n", pthread_self(), i);
		sleep(1);
	}
}

void *
foo2(void *arg)
{
	printf("thread %p, &i = %p\n", pthread_self(), &i);
	for (i = 10; i > 0; i--) {
		printf("thread %p, i = %d\n", pthread_self(), i);
		sleep(1);
	}
}

int main(int argc, char** argv)
{
	pthread_t t1, t2;

	pthread_create(&t1, 0, foo1, 0);
	pthread_create(&t2, 0, foo2, 0);
	pthread_join(t1, 0);
	pthread_join(t2, 0);
}