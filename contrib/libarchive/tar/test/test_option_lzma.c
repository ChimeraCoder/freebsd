
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

DEFINE_TEST(test_option_lzma)
{
	char *p;
	int r;
	size_t s;

	/* Create a file. */
	assertMakeFile("f", 0644, "a");

	/* Archive it with lzma compression. */
	r = systemf("%s -cf - --lzma f >archive.out 2>archive.err",
	    testprog);
	p = slurpfile(&s, "archive.err");
	p[s] = '\0';
	if (r != 0) {
		if (strstr(p, "Unsupported compression") != NULL) {
			skipping("This version of bsdtar was compiled "
			    "without lzma support");
			return;
		}
		failure("--lzma option is broken");
		assertEqualInt(r, 0);
		return;
	}
	/* Check that the archive file has an lzma signature. */
	p = slurpfile(&s, "archive.out");
	assert(s > 2);
	assertEqualMem(p, "\x5d\00\00", 3);
}