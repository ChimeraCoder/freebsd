
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
__FBSDID("$FreeBSD: head/lib/libarchive/test/test_read_file_nonexistent.c 189473 2009-03-07 02:09:21Z kientzle $");

static void read_test(const char *name);
static void write_test(void);

static void
read_test(const char *name)
{
	struct archive* a = archive_read_new();
	int r;

	r = archive_read_support_filter_bzip2(a);
	if((ARCHIVE_WARN == r && !canBzip2()) || ARCHIVE_WARN > r) {
		skipping("bzip2 unsupported");
		return;
	}

	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));

	extract_reference_file(name);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_filename(a, name, 2));
	/* bzip2 and none */
	assertEqualInt(2, archive_filter_count(a));
	
	archive_read_free(a);
}

static void
write_test(void)
{
	char buff[4096];
	struct archive* a = archive_write_new();
	int r;

	assertEqualIntA(a, ARCHIVE_OK, archive_write_set_format_ustar(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_write_set_bytes_per_block(a, 10));

	r = archive_write_add_filter_bzip2(a);
	if((ARCHIVE_WARN == r && !canBzip2()) || ARCHIVE_WARN > r) {
		skipping("bzip2 unsupported");
		return;
	}
	assertEqualIntA(a, ARCHIVE_OK, archive_write_open_memory(a, buff, 4096, 0));
	/* bzip2 and none */
	assertEqualInt(2, archive_filter_count(a));
	archive_write_free(a);
}

DEFINE_TEST(test_filter_count)
{
	read_test("test_compat_bzip2_1.tbz");
	write_test();
}