
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
 * Test that "--help", "-h", and "-W help" options all work and
 * generate reasonable output.
 */

static int
in_first_line(const char *p, const char *substring)
{
	size_t l = strlen(substring);

	while (*p != '\0' && *p != '\n') {
		if (memcmp(p, substring, l) == 0)
			return (1);
		++p;
	}
	return (0);
}

DEFINE_TEST(test_option_help)
{
	int r;
	char *p;
	size_t plen;

	/* Exercise --help option. */
	r = systemf("%s --help >help.stdout 2>help.stderr", testprog);
	assertEqualInt(r, 0);
	failure("--help should generate nothing to stderr.");
	assertEmptyFile("help.stderr");
	/* Help message should start with name of program. */
	p = slurpfile(&plen, "help.stdout");
	failure("Help output should be long enough.");
	assert(plen >= 7);
	failure("First line of help output should contain string 'bsdcpio'");
	assert(in_first_line(p, "bsdcpio"));
	/*
	 * TODO: Extend this check to further verify that --help output
	 * looks approximately right.
	 */
	free(p);

	/* -h option should generate the same output. */
	r = systemf("%s -h >h.stdout 2>h.stderr", testprog);
	assertEqualInt(r, 0);
	failure("-h should generate nothing to stderr.");
	assertEmptyFile("h.stderr");
	failure("stdout should be same for -h and --help");
	assertEqualFile("h.stdout", "help.stdout");

	/* -W help should be another synonym. */
	r = systemf("%s -W help >Whelp.stdout 2>Whelp.stderr", testprog);
	assertEqualInt(r, 0);
	failure("-W help should generate nothing to stderr.");
	assertEmptyFile("Whelp.stderr");
	failure("stdout should be same for -W help and --help");
	assertEqualFile("Whelp.stdout", "help.stdout");
}