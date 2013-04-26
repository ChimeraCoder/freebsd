
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
#include <sys/eui64.h>
#include <stdio.h>
#include <string.h>

#include "test-eui64.h"

static int
test_line(const char *line, const struct eui64 *eui, const char *host)
{
	struct eui64	e;
	char		buf[256];
	static int	test = 0;

	test++;

	if (eui64_line(line, &e, buf, sizeof(buf)) != 0 ||
	    memcmp(&e, eui, sizeof(struct eui64)) != 0 ||
	    strcmp(buf, host) != 0) {
		printf("not ok %d - eui64_line(\"%s\")\n", test, line);
		printf("# host = %s\n", buf);
		eui64_ntoa(&e, buf, sizeof(buf));
		printf("# e = %s\n", buf);
		return (0);
	} else {
		printf("ok %d - eui64_line(\"%s\")\n", test, line);
		return (1);
	}
}

int
main(int argc, char **argv)
{

	printf("1..6\n");

	test_line(test_eui64_line_id, &test_eui64_id,
	    test_eui64_id_host);
	test_line(test_eui64_line_id_colon, &test_eui64_id,
	    test_eui64_id_host);
	test_line(test_eui64_line_eui48, &test_eui64_eui48,
	    test_eui64_eui48_host);
	test_line(test_eui64_line_mac48, &test_eui64_mac48,
	    test_eui64_mac48_host);
	test_line(test_eui64_line_eui48_6byte, &test_eui64_eui48,
	    test_eui64_eui48_host);
	test_line(test_eui64_line_eui48_6byte_c, &test_eui64_eui48,
	    test_eui64_eui48_host);

	return (0);
}