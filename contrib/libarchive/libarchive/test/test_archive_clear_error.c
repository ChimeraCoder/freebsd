
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

DEFINE_TEST(test_archive_clear_error)
{
	struct archive* a = archive_read_new();

	archive_set_error(a, 12, "abcdefgh");
	assertEqualInt(12, archive_errno(a));
	assertEqualString("abcdefgh", archive_error_string(a));

	archive_clear_error(a);
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	archive_read_free(a);
}