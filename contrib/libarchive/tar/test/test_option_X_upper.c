
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

DEFINE_TEST(test_option_X_upper)
{
	int r;

	/*
	 * Create a sample archive.
	 */
	assertMakeFile("file1", 0644, "file1");
	assertMakeFile("file2", 0644, "file2");
	assertMakeFile("file3a", 0644, "file3a");
	assertMakeFile("file4a", 0644, "file4a");
	assertEqualInt(0,
	    systemf("%s -cf archive.tar file1 file2 file3a file4a", testprog));

	/*
	 * Now, try extracting from the test archive with various -X usage.
	 */

	/* Test 1: Without -X */
	assertMakeDir("test1", 0755);
	assertChdir("test1");
	r = systemf("%s -xf ../archive.tar >test.out 2>test.err",
	    testprog);
	if (!assertEqualInt(0, r))
		return;

	assertFileContents("file1", 5, "file1");
	assertFileContents("file2", 5, "file2");
	assertFileContents("file3a", 6, "file3a");
	assertFileContents("file4a", 6, "file4a");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 2: Use -X to skip one file */
	assertMakeDir("test2", 0755);
	assertChdir("test2");
	assertMakeFile("exclusions", 0644, "file1\n");
	assertEqualInt(0,
	    systemf("%s -xf ../archive.tar -X exclusions >test.out 2>test.err", testprog));
	assertFileNotExists("file1");
	assertFileContents("file2", 5, "file2");
	assertFileContents("file3a", 6, "file3a");
	assertFileContents("file4a", 6, "file4a");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 3: Use -X to skip multiple files */
	assertMakeDir("test3", 0755);
	assertChdir("test3");
	assertMakeFile("exclusions", 0644, "file1\nfile2\n");
	assertEqualInt(0,
	    systemf("%s -xf ../archive.tar -X exclusions >test.out 2>test.err", testprog));
	assertFileNotExists("file1");
	assertFileNotExists("file2");
	assertFileContents("file3a", 6, "file3a");
	assertFileContents("file4a", 6, "file4a");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 4: Omit trailing \n */
	assertMakeDir("test4", 0755);
	assertChdir("test4");
	assertMakeFile("exclusions", 0644, "file1\nfile2");
	assertEqualInt(0,
	    systemf("%s -xf ../archive.tar -X exclusions >test.out 2>test.err", testprog));
	assertFileNotExists("file1");
	assertFileNotExists("file2");
	assertFileContents("file3a", 6, "file3a");
	assertFileContents("file4a", 6, "file4a");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 5: include/exclude without overlap */
	assertMakeDir("test5", 0755);
	assertChdir("test5");
	assertMakeFile("exclusions", 0644, "file1\nfile2");
	assertEqualInt(0,
	    systemf("%s -xf ../archive.tar -X exclusions file3a >test.out 2>test.err", testprog));
	assertFileNotExists("file1");
	assertFileNotExists("file2");
	assertFileContents("file3a", 6, "file3a");
	assertFileNotExists("file4a");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 6: Overlapping include/exclude */
	assertMakeDir("test6", 0755);
	assertChdir("test6");
	assertMakeFile("exclusions", 0644, "file1\nfile2");
	assertEqualInt(0,
	    systemf("%s -xf ../archive.tar -X exclusions file1 file3a >test.out 2>test.err", testprog));
	assertFileNotExists("file1");
	assertFileNotExists("file2");
	assertFileContents("file3a", 6, "file3a");
	assertFileNotExists("file4a");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 7: with pattern */
	assertMakeDir("test7", 0755);
	assertChdir("test7");
	assertMakeFile("exclusions", 0644, "file*a\nfile1");
	assertEqualInt(0,
	    systemf("%s -xf ../archive.tar -X exclusions >test.out 2>test.err", testprog));
	assertFileNotExists("file1");
	assertFileContents("file2", 5, "file2");
	assertFileNotExists("file3a");
	assertFileNotExists("file4a");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");
}