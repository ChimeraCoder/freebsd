
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
/* try to catch thread exiting, and rethrow the exception */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int caught;

void *
thr_routine(void *arg)
{
	try {
		pthread_exit(NULL);
	} catch (...) {
		caught = 1;
		printf("thread exiting exception caught\n");
		/* rethrow */
		throw;
	}
}

int
main()
{
	pthread_t td;

	pthread_create(&td, NULL, thr_routine, NULL);
	pthread_join(td, NULL);
	if (caught)
		printf("OK\n");
	else
		printf("failure\n");
	return (0);
}