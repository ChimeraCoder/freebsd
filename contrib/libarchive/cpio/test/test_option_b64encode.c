
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

DEFINE_TEST(test_option_b64encode)
{
	char *p;
	size_t s;

	/* Create a file. */
	assertMakeFile("f", 0644, "a");

	/* Archive it with compress compression and uuencode. */
	assertEqualInt(0,
	    systemf("echo f | %s -o -Z --b64encode >archive.out 2>archive.err",
	    testprog));
	/* Check that the archive file has an uuencode signature. */
	p = slurpfile(&s, "archive.out");
	assert(s > 2);
	assertEqualMem(p, "begin-base64 644", 16);

	/* Archive it with uuencode only. */
	assertEqualInt(0,
	    systemf("echo f | %s -o --b64encode >archive.out 2>archive.err",
	    testprog));
	/* Check that the archive file has an uuencode signature. */
	p = slurpfile(&s, "archive.out");
	assert(s > 2);
	assertEqualMem(p, "begin-base64 644", 16);
}