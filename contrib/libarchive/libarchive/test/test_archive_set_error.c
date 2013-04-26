
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

static void
test(struct archive *a, int code, const char *msg)
{
	archive_set_error(a, code, "%s", msg);

	assertEqualInt(code, archive_errno(a));
	assertEqualString(msg, archive_error_string(a));
}

DEFINE_TEST(test_archive_set_error)
{
	struct archive* a = archive_read_new();

	/* unlike printf("%s", NULL),
	 * archive_set_error(a, code, "%s", NULL)
	 * segfaults, so it's not tested here */
	test(a, 12, "abcdefgh");
	test(a, 0, "123456");
	test(a, -1, "tuvw");
	test(a, 34, "XYZ");

	archive_read_free(a);
}