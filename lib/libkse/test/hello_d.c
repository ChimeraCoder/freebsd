
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
/****************************************************************************
 *
 * Simple diff mode test.
 *
 * $FreeBSD$
 *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <pthread.h>

void *
entry(void * a_arg)
{
	fprintf(stderr, "Hello world\n");

	return NULL;
}

int
main()
{
	pthread_t thread;
	int error;

	error = pthread_create(&thread, NULL, entry, NULL);
	if (error)
		fprintf(stderr, "Error in pthread_create(): %s\n",
			strerror(error));

	error = pthread_join(thread, NULL);
	if (error)
		fprintf(stderr, "Error in pthread_join(): %s\n",
			strerror(error));

	return 0;
}