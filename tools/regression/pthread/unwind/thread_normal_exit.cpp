
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
/* test stack unwinding for a new thread */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "Test.cpp"

void *
thr_routine(void *arg)
{
	Test test;

	pthread_exit(NULL);
	printf("Bug, thread shouldn't be here\n");
}

int
main()
{
	pthread_t td;

	pthread_create(&td, NULL, thr_routine, NULL);
	pthread_join(td, NULL);
	check_destruct();
	return (0);
}