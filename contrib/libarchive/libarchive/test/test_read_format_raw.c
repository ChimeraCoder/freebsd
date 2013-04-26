
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

DEFINE_TEST(test_read_format_raw)
{
	char buff[512];
	struct archive_entry *ae;
	struct archive *a;
	const char *reffile1 = "test_read_format_raw.data";
	const char *reffile2 = "test_read_format_raw.data.Z";

	/* First, try pulling data out of an uninterpretable file. */
	extract_reference_file(reffile1);
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_raw(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_open_filename(a, reffile1, 512));

	/* First (and only!) Entry */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("data", archive_entry_pathname(ae));
	/* Most fields should be unset (unknown) */
	assert(!archive_entry_size_is_set(ae));
	assert(!archive_entry_atime_is_set(ae));
	assert(!archive_entry_ctime_is_set(ae));
	assert(!archive_entry_mtime_is_set(ae));
	assertEqualInt(4, archive_read_data(a, buff, 32));
	assertEqualMem(buff, "foo\n", 4);

	/* Test EOF */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));


	/* Second, try the same with a compressed file. */
	extract_reference_file(reffile2);
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_raw(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_open_filename(a, reffile2, 1));

	/* First (and only!) Entry */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("data", archive_entry_pathname(ae));
	/* Most fields should be unset (unknown) */
	assert(!archive_entry_size_is_set(ae));
	assert(!archive_entry_atime_is_set(ae));
	assert(!archive_entry_ctime_is_set(ae));
	assert(!archive_entry_mtime_is_set(ae));
	assertEqualInt(4, archive_read_data(a, buff, 32));
	assertEqualMem(buff, "foo\n", 4);

	/* Test EOF */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}