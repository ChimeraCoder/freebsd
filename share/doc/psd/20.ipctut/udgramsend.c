
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
 * Here I send a datagram to a receiver whose name I get from the command
 * line arguments.  The form of the command line is udgramsend pathname
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	int sock;
	struct sockaddr_un name;

	/* Create socket on which to send. */
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("opening datagram socket");
		exit(1);
	}
	/* Construct name of socket to send to. */
	name.sun_family = AF_UNIX;
	strcpy(name.sun_path, argv[1]);
	/* Send message. */
	if (sendto(sock, DATA, sizeof(DATA), 0,
	    &name, sizeof(struct sockaddr_un)) < 0) {
		perror("sending datagram message");
	}
	close(sock);
}