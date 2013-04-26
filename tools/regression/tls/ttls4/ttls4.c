
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
 * This program tests if a new thread's initial tls data
 * is clean.
 *
 * David Xu <davidxu@freebsd.org>
 *
 * $FreeBSD$
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int __thread n;

void *f1(void *arg)
{
	if (n != 0) {
		printf("bug, n == %d \n", n);
		exit(1);
	}
	n = 1;
	return (0);
}

int main()
{
	pthread_t td;
	int i;

	for (i = 0; i < 1000; ++i) {
		pthread_create(&td, NULL, f1, NULL);
		pthread_join(td, NULL);
	}
	sleep(2);
	for (i = 0; i < 1000; ++i) {
		pthread_create(&td, NULL, f1, NULL);
		pthread_join(td, NULL);
	}
	return (0);
}