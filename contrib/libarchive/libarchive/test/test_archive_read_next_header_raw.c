
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

#define DATA "random garbage for testing purposes"

static const char data[sizeof(DATA)] = DATA;

static void
test(int skip_explicitely)
{
	struct archive* a = archive_read_new();
	struct archive_entry* e;

	assertEqualInt(ARCHIVE_OK, archive_read_support_format_raw(a));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	assertEqualInt(ARCHIVE_OK, archive_read_open_memory(a,
	    (void *)(uintptr_t) data, sizeof(data)));
	assertEqualString(NULL, archive_error_string(a));

	assertEqualInt(ARCHIVE_OK, archive_read_next_header(a, &e));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	if (skip_explicitely)
		assertEqualInt(ARCHIVE_OK, archive_read_data_skip(a));

	assertEqualInt(ARCHIVE_EOF, archive_read_next_header(a, &e));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	archive_read_free(a);
}

DEFINE_TEST(test_archive_read_next_header_raw)
{
	test(1);
	test(0);
}