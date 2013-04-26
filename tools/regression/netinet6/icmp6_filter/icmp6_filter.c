
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
 * This regression test creates a raw IPv6 socket and confirms that it can
 * set and get filters on the socket.  No attempt is made to validate that
 * the filter is implemented, just that it can be properly retrieved, set,
 * etc.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/icmp6.h>

#include <err.h>
#include <string.h>
#include <unistd.h>

/*
 * Reference filters to set/test.
 */
static struct icmp6_filter ic6f_passall;
static struct icmp6_filter ic6f_blockall;

int
main(int argc, char *argv[])
{
	struct icmp6_filter ic6f;
	socklen_t len;
	int s;

	ICMP6_FILTER_SETPASSALL(&ic6f_passall);
	ICMP6_FILTER_SETBLOCKALL(&ic6f_blockall);

	s = socket(PF_INET6, SOCK_RAW, 0);
	if (s < 0)
		err(-1, "socket(PF_INET6, SOCK_RAW, 0)");

	/*
	 * Confirm that we can read before the first set, and that the
	 * default is to pass all ICMP.
	 */
	len = sizeof(ic6f);
	if (getsockopt(s, IPPROTO_ICMPV6, ICMP6_FILTER, &ic6f, &len) < 0)
		err(-1, "1: getsockopt(ICMP6_FILTER)");
	if (memcmp(&ic6f, &ic6f_passall, sizeof(ic6f)) != 0)
		errx(-1, "1: getsockopt(ICMP6_FILTER) - default not passall");

	/*
	 * Confirm that we can write a pass all filter to the socket.
	 */
	len = sizeof(ic6f);
	ICMP6_FILTER_SETPASSALL(&ic6f);
	if (setsockopt(s, IPPROTO_ICMPV6, ICMP6_FILTER, &ic6f, len) < 0)
		err(-1, "2: setsockopt(ICMP6_FILTER, PASSALL)");

	/*
	 * Confirm that we can still read a pass all filter.
	 */
	len = sizeof(ic6f);
	if (getsockopt(s, IPPROTO_ICMPV6, ICMP6_FILTER, &ic6f, &len) < 0)
		err(-1, "3: getsockopt(ICMP6_FILTER)");
	if (memcmp(&ic6f, &ic6f_passall, sizeof(ic6f)) != 0)
		errx(-1, "3: getsockopt(ICMP6_FILTER) - not passall");

	/*
	 * Confirm that we can write a block all filter to the socket.
	 */
	len = sizeof(ic6f);
	ICMP6_FILTER_SETBLOCKALL(&ic6f);
	if (setsockopt(s, IPPROTO_ICMPV6, ICMP6_FILTER, &ic6f, len) < 0)
		err(-1, "4: setsockopt(ICMP6_FILTER, BLOCKALL)");

	/*
	 * Confirm that we can read back a block all filter.
	 */
	len = sizeof(ic6f);
	if (getsockopt(s, IPPROTO_ICMPV6, ICMP6_FILTER, &ic6f, &len) < 0)
		err(-1, "5: getsockopt(ICMP6_FILTER)");
	if (memcmp(&ic6f, &ic6f_blockall, sizeof(ic6f)) != 0)
		errx(-1, "5: getsockopt(ICMP6_FILTER) - not blockall");

	/*
	 * For completeness, confirm that we can reset to the default.
	 */
	len = sizeof(ic6f);
	ICMP6_FILTER_SETPASSALL(&ic6f);
	if (setsockopt(s, IPPROTO_ICMPV6, ICMP6_FILTER, &ic6f, len) < 0)
		err(-1, "6: setsockopt(ICMP6_FILTER, PASSALL)");

	/*
	 * ... And that we can read back the pass all rule again.
	 */
	len = sizeof(ic6f);
	if (getsockopt(s, IPPROTO_ICMPV6, ICMP6_FILTER, &ic6f, &len) < 0)
		err(-1, "7: getsockopt(ICMP6_FILTER)");
	if (memcmp(&ic6f, &ic6f_passall, sizeof(ic6f)) != 0)
		errx(-1, "7: getsockopt(ICMP6_FILTER) - not passall");

	close(s);
	return (0);
}