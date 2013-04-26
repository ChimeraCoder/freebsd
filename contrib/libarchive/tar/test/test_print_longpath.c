
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

DEFINE_TEST(test_print_longpath)
{
	const char *reffile = "test_print_longpath.tar.Z";
	char buff[2048];
	int i, j, k;

	/* Reference file has one entry "file" with a very old timestamp. */
	extract_reference_file(reffile);

	/* Build long path pattern. */
	memset(buff, 0, sizeof(buff));
	for (k = 0; k < 4; k++) {
		for (j = 0; j < k+1; j++) {
			for (i = 0; i < 10; i++)
				strncat(buff, "0123456789",
				    sizeof(buff) - strlen(buff) -1);
			strncat(buff, "/", sizeof(buff) - strlen(buff) -1);
		}
		strncat(buff, "\n", sizeof(buff) - strlen(buff) -1);
	}
	buff[sizeof(buff)-1] = '\0';

	assertEqualInt(0,
	    systemf("%s -tf %s >test.out 2>test.err", testprog, reffile));
	assertTextFileContents(buff, "test.out");
	assertEmptyFile("test.err");
}