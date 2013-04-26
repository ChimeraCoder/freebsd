
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <pthread.h>

#define	NITERATIONS	16384
#define	MAXGROWTH	16384

void *
thread_entry(void *a_arg)
{
	return NULL;
}

int
main(void)
{
	pthread_t	thread;
	int		i, error;
	char		*brk, *nbrk;
	unsigned	growth;

	fprintf(stderr, "Test begin\n");

	/* Get an initial brk value. */
	brk = sbrk(0);

	/* Create threads and join them, one at a time. */
	for (i = 0; i < NITERATIONS; i++) {
		if ((error = pthread_create(&thread, NULL, thread_entry, NULL))
		    != 0) {
			fprintf(stderr, "Error in pthread_create(): %s\n",
			    strerror(error));
			exit(1);
		}
		if ((error = pthread_join(thread, NULL)) != 0) {
			fprintf(stderr, "Error in pthread_join(): %s\n",
			    strerror(error));
			exit(1);
		}
	}

	/* Get a final brk value. */
	nbrk = sbrk(0);

	/*
	 * Check that the amount of heap space allocated is below an acceptable
	 * threshold.  We could just compare brk and nbrk, but the test could
	 * conceivably break if the internals of the threads library changes.
	 */
	if (nbrk > brk) {
		/* Heap grows up. */
		growth = nbrk - brk;
	} else if (nbrk <= brk) {
		/* Heap grows down, or no growth. */
		growth = brk - nbrk;
	}

	if (growth > MAXGROWTH) {
		fprintf(stderr, "Heap growth exceeded maximum (%u > %u)\n",
		    growth, MAXGROWTH);
	}
#if (0)
	else {
		fprintf(stderr, "Heap growth acceptable (%u <= %u)\n",
		    growth, MAXGROWTH);
	}
#endif

	fprintf(stderr, "Test end\n");
	return 0;
}