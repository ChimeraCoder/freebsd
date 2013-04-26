
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


DEFINE_TEST(test_option_B_upper)
{
	struct stat st;
	int r;

	/*
	 * Create a file on disk.
	 */
	assertMakeFile("file", 0644, NULL);

	/* Create an archive without -B; this should be 512 bytes. */
	r = systemf("echo file | %s -o > small.cpio 2>small.err", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "small.err");
	assertEqualInt(0, stat("small.cpio", &st));
	assertEqualInt(512, st.st_size);

	/* Create an archive with -B; this should be 5120 bytes. */
	r = systemf("echo file | %s -oB > large.cpio 2>large.err", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "large.err");
	assertEqualInt(0, stat("large.cpio", &st));
	assertEqualInt(5120, st.st_size);
}