
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
#include <fcntl.h>

/*
 * $FreeBSD$
 * This program tests how sys_pipe.c handles the case where there
 * is ample memory to allocate a pipe, but the file descriptor
 * limit for that user has been exceeded.
 */

int main (int argc, void *argv[])

{
	int i, returnval, lastfd;
	int pipes[10000];

	for (i = 0; i < 100000; i++) {
		returnval = open(argv[0], O_RDONLY);
		if (returnval < 1)
			break; /* All descriptors exhausted. */
		else
			lastfd = returnval;
	}

	/* First falloc failure case in sys_pipe.c:pipe() */
	for (i = 0; i < 1000; i++) {
		returnval = pipe(&pipes[i]);
	}

	/*
	 * Free just one FD so that the second falloc failure
	 * case will occur.
	 */
	close(lastfd);

	for (i = 0; i < 1000; i++) {
		returnval = pipe(&pipes[i]);
	}
	printf("PASS\n");
}