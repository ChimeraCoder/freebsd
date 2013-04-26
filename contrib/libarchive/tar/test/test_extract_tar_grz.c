
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

DEFINE_TEST(test_extract_tar_grz)
{
	const char *reffile = "test_extract.tar.grz";
	int f;

	extract_reference_file(reffile);
	f = systemf("%s -tf %s >test.out 2>test.err", testprog, reffile);
	if (f == 0 || canGrzip()) {
		assertEqualInt(0, systemf("%s -xf %s >test.out 2>test.err",
		    testprog, reffile));

		assertFileExists("file1");
		assertTextFileContents("contents of file1.\n", "file1");
		assertFileExists("file2");
		assertTextFileContents("contents of file2.\n", "file2");
		assertEmptyFile("test.out");
		assertEmptyFile("test.err");
	} else {
		skipping("It seems grzip is not supported on this platform");
	}
}