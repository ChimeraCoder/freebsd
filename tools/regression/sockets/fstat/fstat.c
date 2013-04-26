
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <err.h>
#include <unistd.h>

/*
 * Basic test to make sure that fstat(2) returns success on various socket
 * types.  In the future we should also validate the fields, confirming
 * expected results such as the effect of shutdown(2) on permissions, etc.
 */

static void
dotest(int domain, int type, int protocol)
{
	struct stat sb;
	int sock;

	sock = socket(domain, type, protocol);
	if (sock < 0)
		err(-1, "socket(%d, %d, %d)", domain, type, protocol);

	if (fstat(sock, &sb) < 0)
		err(-1, "fstat on socket(%d, %d, %d)", domain, type,
		    protocol);

	close(sock);
}

int
main(int argc, char *argv[])
{

	dotest(PF_INET, SOCK_DGRAM, 0);
	dotest(PF_INET, SOCK_STREAM, 0);
	dotest(PF_INET6, SOCK_DGRAM, 0);
	dotest(PF_INET6, SOCK_STREAM, 0);
	dotest(PF_LOCAL, SOCK_DGRAM, 0);
	dotest(PF_LOCAL, SOCK_STREAM, 0);

	return (0);
}