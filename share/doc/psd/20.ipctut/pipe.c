
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
 * This program creates a pipe, then forks.  The child communicates to the
 * parent over the pipe. Notice that a pipe is a one-way communications
 * device.  I can write to the output socket (sockets[1], the second socket
 * of the array returned by pipe()) and read from the input socket
 * (sockets[0]), but not vice versa.
 */
main()
{
	int sockets[2], child;

	/* Create a pipe */
	if (pipe(sockets) < 0) {
		perror("opening stream socket pair");
		exit(10);
	}

	if ((child = fork()) == -1)
		perror("fork");
	else if (child) {
		char buf[1024];

		/* This is still the parent.  It reads the child's message. */
		close(sockets[1]);
		if (read(sockets[0], buf, 1024) < 0)
			perror("reading message");
		printf("-->%s\en", buf);
		close(sockets[0]);
	} else {
		/* This is the child.  It writes a message to its parent. */
		close(sockets[0]);
		if (write(sockets[1], DATA, sizeof(DATA)) < 0)
			perror("writing message");
		close(sockets[1]);
	}
}