
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

/*
 * Regression test:  We used to get a bogus error message when we
 * asked tar to copy entries out of an empty archive.  See
 * Issue 51 on libarchive.googlecode.com for details.
 */
DEFINE_TEST(test_empty_mtree)
{
	int r;

	assertMakeFile("test1.mtree", 0777, "#mtree\n");

	r = systemf("%s cf test1.tar @test1.mtree >test1.out 2>test1.err",
	    testprog);
	failure("Error invoking %s cf", testprog);
	assertEqualInt(r, 0);
	assertEmptyFile("test1.out");
	assertEmptyFile("test1.err");
}