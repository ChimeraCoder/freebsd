
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

DEFINE_TEST(test_option_keep_newer_files)
{
	const char *reffile = "test_option_keep_newer_files.tar.Z";

	/* Reference file has one entry "file" with a very old timestamp. */
	extract_reference_file(reffile);

	/* Test 1: Without --keep-newer-files */
	assertMakeDir("test1", 0755);
	assertChdir("test1");
	assertMakeFile("file", 0644, "new");
	assertEqualInt(0,
	    systemf("%s -xf ../%s >test.out 2>test.err", testprog, reffile));
	assertFileContents("old\n", 4, "file");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 2: With --keep-newer-files */
	assertMakeDir("test2", 0755);
	assertChdir("test2");
	assertMakeFile("file", 0644, "new");
	assertEqualInt(0,
	    systemf("%s -xf ../%s --keep-newer-files >test.out 2>test.err", testprog, reffile));
	assertFileContents("new", 3, "file");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");
}