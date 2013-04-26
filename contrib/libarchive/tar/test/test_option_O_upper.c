
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

static const char *test4out[] = {"file1", "file2", NULL};
static const char *test5err[] = {"file1", "file2", NULL};

DEFINE_TEST(test_option_O_upper)
{
	assertMakeFile("file1", 0644, "file1");
	assertMakeFile("file2", 0644, "file2");
	assertEqualInt(0, systemf("%s -cf archive.tar file1 file2", testprog));

	/* Test 1: -x without -O */
	assertMakeDir("test1", 0755);
	assertChdir("test1");
	assertEqualInt(0,
	    systemf("%s -xf ../archive.tar >test.out 2>test.err", testprog));
	assertFileContents("file1", 5, "file1");
	assertFileContents("file2", 5, "file2");
	assertEmptyFile("test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 2: -x with -O */
	assertMakeDir("test2", 0755);
	assertChdir("test2");
	assertEqualInt(0,
	    systemf("%s -xOf ../archive.tar file1 >test.out 2>test.err", testprog));
	assertFileNotExists("file1");
	assertFileNotExists("file2");
	assertFileContents("file1", 5, "test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 3: -x with -O and multiple files */
	assertMakeDir("test3", 0755);
	assertChdir("test3");
	assertEqualInt(0,
	    systemf("%s -xOf ../archive.tar >test.out 2>test.err", testprog));
	assertFileNotExists("file1");
	assertFileNotExists("file2");
	assertFileContents("file1file2", 10, "test.out");
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 4: -t without -O */
	assertMakeDir("test4", 0755);
	assertChdir("test4");
	assertEqualInt(0,
	    systemf("%s -tf ../archive.tar >test.out 2>test.err", testprog));
	assertFileContainsLinesAnyOrder("test.out", test4out);
	assertEmptyFile("test.err");
	assertChdir("..");

	/* Test 5: -t with -O */
	assertMakeDir("test5", 0755);
	assertChdir("test5");
	assertEqualInt(0,
	    systemf("%s -tOf ../archive.tar >test.out 2>test.err", testprog));
	assertEmptyFile("test.out");
	assertFileContainsLinesAnyOrder("test.err", test5err);
	assertChdir("..");
}