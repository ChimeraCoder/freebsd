
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

DEFINE_TEST(test_option_H_upper)
{

	if (!canSymlink()) {
		skipping("Can't test symlinks on this filesystem");
		return;
	}

	/*
	 * Create a sample archive.
	 */
	assertMakeDir("in", 0755);
	assertChdir("in");
	assertMakeDir("d1", 0755);
	assertMakeSymlink("ld1", "d1");
	assertMakeFile("d1/file1", 0644, "d1/file1");
	assertMakeFile("d1/file2", 0644, "d1/file2");
	assertMakeSymlink("d1/link1", "file1");
	assertMakeSymlink("d1/linkX", "fileX");
	assertMakeSymlink("link2", "d1/file2");
	assertMakeSymlink("linkY", "d1/fileY");
	assertChdir("..");

	/* Test 1: Without -H */
	assertMakeDir("test1", 0755);
	assertEqualInt(0,
	    systemf("%s -cf test1/archive.tar -C in . >test1/c.out 2>test1/c.err", testprog));
	assertChdir("test1");
	assertEqualInt(0,
	    systemf("%s -xf archive.tar >c.out 2>c.err", testprog));
	assertIsSymlink("ld1", "d1");
	assertIsSymlink("d1/link1", "file1");
	assertIsSymlink("d1/linkX", "fileX");
	assertIsSymlink("link2", "d1/file2");
	assertIsSymlink("linkY", "d1/fileY");
	assertChdir("..");

	/* Test 2: With -H, no symlink on command line. */
	assertMakeDir("test2", 0755);
	assertEqualInt(0,
	    systemf("%s -cf test2/archive.tar -H -C in . >test2/c.out 2>test2/c.err", testprog));
	assertChdir("test2");
	assertEqualInt(0,
	    systemf("%s -xf archive.tar >c.out 2>c.err", testprog));
	assertIsSymlink("ld1", "d1");
	assertIsSymlink("d1/link1", "file1");
	assertIsSymlink("d1/linkX", "fileX");
	assertIsSymlink("link2", "d1/file2");
	assertIsSymlink("linkY", "d1/fileY");
	assertChdir("..");

	/* Test 3: With -H, some symlinks on command line. */
	assertMakeDir("test3", 0755);
	assertEqualInt(0,
	    systemf("%s -cf test3/archive.tar -H -C in ld1 d1 link2 linkY >test2/c.out 2>test2/c.err", testprog));
	assertChdir("test3");
	assertEqualInt(0,
	    systemf("%s -xf archive.tar >c.out 2>c.err", testprog));
	assertIsDir("ld1", 0755);
	assertIsSymlink("d1/linkX", "fileX");
	assertIsSymlink("d1/link1", "file1");
	assertIsReg("link2", 0644);
	assertIsSymlink("linkY", "d1/fileY");
	assertChdir("..");
}