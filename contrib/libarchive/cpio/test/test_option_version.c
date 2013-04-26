
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
#include "test.h"
__FBSDID("$FreeBSD$");

/*
 * Test that --version option works and generates reasonable output.
 */

static void
verify(const char *p, size_t s)
{
	const char *q = p;

	/* Version message should start with name of program, then space. */
	failure("version message too short:", p);
	if (!assert(s > 6))
		return;
	failure("Version message should begin with 'bsdcpio': %s", p);
	if (!assertEqualMem(q, "bsdcpio ", 8))
		/* If we're not testing bsdcpio, don't keep going. */
		return;
	q += 8; s -= 8;
	/* Version number is a series of digits and periods. */
	while (s > 0 && (*q == '.' || (*q >= '0' && *q <= '9'))) {
		++q;
		--s;
	}
	/* Version number terminated by space. */
	failure("Version: %s", p);
	assert(s > 1);
	/* Skip a single trailing a,b,c, or d. */
	if (*q == 'a' || *q == 'b' || *q == 'c' || *q == 'd')
		++q;
	failure("Version: %s", p);
	assert(*q == ' ');
	++q; --s;
	/* Separator. */
	failure("Version: %s", p);
	assertEqualMem(q, "-- ", 3);
	q += 3; s -= 3;
	/* libarchive name and version number */
	assert(s > 11);
	failure("Version: %s", p);
	assertEqualMem(q, "libarchive ", 11);
	q += 11; s -= 11;
	/* Version number is a series of digits and periods. */
	while (s > 0 && (*q == '.' || (*q >= '0' && *q <= '9'))) {
		++q;
		--s;
	}
	/* Skip a single trailing a,b,c, or d. */
	if (*q == 'a' || *q == 'b' || *q == 'c' || *q == 'd')
		++q;
	/* All terminated by end-of-line: \r, \r\n, or \n */
	assert(s >= 1);
	failure("Version: %s", p);
	if (*q == '\x0d') {
		if (q[1] != '\0')
			assertEqualMem(q, "\x0d\x0a", 2);
	} else
		assertEqualMem(q, "\x0a", 1);
}


DEFINE_TEST(test_option_version)
{
	int r;
	char *p;
	size_t s;

	r = systemf("%s --version >version.stdout 2>version.stderr", testprog);
	if (r != 0)
		r = systemf("%s -W version >version.stdout 2>version.stderr",
		    testprog);
	failure("Unable to run either %s --version or %s -W version",
	    testprog, testprog);
	if (!assert(r == 0))
		return;

	/* --version should generate nothing to stderr. */
	assertEmptyFile("version.stderr");
	/* Verify format of version message. */
	p = slurpfile(&s, "version.stdout");
	verify(p, s);
	free(p);
}