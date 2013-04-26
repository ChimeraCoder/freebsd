
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

DEFINE_TEST(test_option_n)
{
	assertMakeDir("d1", 0755);
	assertMakeFile("d1/file1", 0644, "d1/file1");

	/* Test 1: -c without -n */
	assertMakeDir("test1", 0755);
	assertChdir("test1");
	assertEqualInt(0,
	    systemf("%s -cf archive.tar -C .. d1 >c.out 2>c.err", testprog));
	assertEmptyFile("c.out");
	assertEmptyFile("c.err");
	assertEqualInt(0,
	    systemf("%s -xf archive.tar >x.out 2>x.err", testprog));
	assertEmptyFile("x.out");
	assertEmptyFile("x.err");
	assertFileContents("d1/file1", 8, "d1/file1");
	assertChdir("..");

	/* Test 2: -c with -n */
	assertMakeDir("test2", 0755);
	assertChdir("test2");
	assertEqualInt(0,
	    systemf("%s -cnf archive.tar -C .. d1 >c.out 2>c.err", testprog));
	assertEmptyFile("c.out");
	assertEmptyFile("c.err");
	assertEqualInt(0,
	    systemf("%s -xf archive.tar >x.out 2>x.err", testprog));
	assertEmptyFile("x.out");
	assertEmptyFile("x.err");
	assertIsDir("d1", 0755);
	assertFileNotExists("d1/file1");
	assertChdir("..");
}