
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


DEFINE_TEST(test_option_C_upper)
{
	int r;

	/*
	 * Create a file on disk.
	 */
	assertMakeFile("file", 0644, NULL);

	/* Create an archive without -C; this should be 512 bytes. */
	r = systemf("echo file | %s -o > small.cpio 2>small.err", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "small.err");
	assertFileSize("small.cpio", 512);

	/* Create an archive with -C 513; this should be 513 bytes. */
	r = systemf("echo file | %s -o -C 513 > 513.cpio 2>513.err",
		    testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "513.err");
	assertFileSize("513.cpio", 513);

	/* Create an archive with -C 12345; this should be 12345 bytes. */
	r = systemf("echo file | %s -o -C12345 > 12345.cpio 2>12345.err",
		    testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "12345.err");
	assertFileSize("12345.cpio", 12345);

	/* Create an archive with invalid -C request */
	assert(0 != systemf("echo file | %s -o -C > bad.cpio 2>bad.err",
			    testprog));
	assertEmptyFile("bad.cpio");
}