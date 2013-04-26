
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/param.h>
#include <errno.h>
#include <libutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TESTDOMAIN ""
#define TESTHOST "testhost"
#define TESTFQDN "testhost" TESTDOMAIN

int failures = 0;
int tests = 0;

/*
 * Evily override gethostname(3) so trimdomain always gets the same result.
 * This makes the tests much easier to write and less likely to fail on
 * oddly configured systems.
 */
int
gethostname(char *name, size_t namelen)
{
	if (strlcpy(name, TESTFQDN, namelen) > namelen) {
		errno = ENAMETOOLONG;
		return (-1);
	}
	return (0);
}

void
testit(const char *input, int hostsize, const char *output, const char *test)
{
	char *testhost;
	const char *expected = (output == NULL) ? input : output;

	testhost = strdup(input);
	trimdomain(testhost, hostsize < 0 ? (int)strlen(testhost) : hostsize);
	tests++;
	if (strcmp(testhost, expected) != 0) {
		printf("not ok %d - %s\n", tests, test);
		printf("# %s -> %s (expected %s)\n", input, testhost, expected);
	} else
		printf("ok %d - %s\n", tests, test);
	free(testhost);
	return;
}

int
main(void)
{

	printf("1..5\n");

	testit(TESTFQDN, -1, TESTHOST, "self");
	testit("XXX" TESTDOMAIN, -1, "XXX", "different host, same domain");
	testit("XXX" TESTDOMAIN, 1, NULL, "short hostsize");
	testit("bogus.example.net", -1, NULL, "arbitrary host");
	testit("XXX." TESTFQDN, -1, NULL, "domain is local hostname");

	return (0);
}