
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
 * Simple regression test to open and then immediately close various types of
 * PF_IPX sockets.  Run with various waits in order to make sure that the
 * various IPX/SPX timers have a chance to walk the pcb lists and hit the
 * sockets.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netipx/ipx.h>

#include <err.h>
#include <unistd.h>

static int
maybe_sleep(int sec)
{

	if (sec == 0)
		return (0);
	return (sleep(sec));
}

int
main(int argc, char *argv[])
{
	int delay, s;

	for (delay = 0; delay < 5; delay++) {
		s = socket(PF_IPX, SOCK_DGRAM, 0);
		if (s < 0)
			warn("socket(PF_IPX, SOCK_DGRAM, 0)");
		else {
			maybe_sleep(delay);
			close(s);
		}

		s = socket(PF_IPX, SOCK_STREAM, 0);
		if (s < 0)
			warn("socket(PF_IPX, SOCK_STREAM, 0)");
		else {
			maybe_sleep(delay);
			close(s);
		}

		s = socket(PF_IPX, SOCK_SEQPACKET, 0);
		if (s < 0)
			warn("socket(PF_IPX, SOCK_SEQPACKET, 0)");
		else {
			maybe_sleep(delay);
			close(s);
		}

		s = socket(PF_IPX, SOCK_RAW, 0);
		if (s < 0)
			warn("socket(PF_IPX, SOCK_RAW, 0)");
		else {
			maybe_sleep(delay);
			close(s);
		}
	}

	return (0);
}