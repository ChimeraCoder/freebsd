
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

DEFINE_TEST(test_empty_write)
{
	char buff[32768];
	struct archive_entry *ae;
	struct archive *a;
	size_t used;
	int r;

	/*
	 * Exercise a zero-byte write to a gzip-compressed archive.
	 */

	/* Create a new archive in memory. */
	assert((a = archive_write_new()) != NULL);
	assertA(0 == archive_write_set_format_ustar(a));
	r = archive_write_add_filter_gzip(a);
	if (r != ARCHIVE_OK && !canGzip()) {
		skipping("Empty write to gzip-compressed archive");
	} else {
		if (r != ARCHIVE_OK && canGzip())
			assertEqualIntA(a, ARCHIVE_WARN, r);
		else
			assertEqualIntA(a, ARCHIVE_OK, r);
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_open_memory(a, buff, sizeof(buff), &used));
		/* Write a file to it. */
		assert((ae = archive_entry_new()) != NULL);
		archive_entry_copy_pathname(ae, "file");
		archive_entry_set_mode(ae, S_IFREG | 0755);
		archive_entry_set_size(ae, 0);
		assertA(0 == archive_write_header(a, ae));
		archive_entry_free(ae);

		/* THE TEST: write zero bytes to this entry. */
		/* This used to crash. */
		assertEqualIntA(a, 0, archive_write_data(a, "", 0));

		/* Close out the archive. */
		assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
		assertEqualInt(ARCHIVE_OK, archive_write_free(a));
	}

	/*
	 * Again, with bzip2 compression.
	 */

	/* Create a new archive in memory. */
	assert((a = archive_write_new()) != NULL);
	assertA(0 == archive_write_set_format_ustar(a));
	r = archive_write_add_filter_bzip2(a);
	if (r != ARCHIVE_OK && !canBzip2()) {
		skipping("Empty write to bzip2-compressed archive");
	} else {
		if (r != ARCHIVE_OK && canBzip2())
			assertEqualIntA(a, ARCHIVE_WARN, r);
		else
			assertEqualIntA(a, ARCHIVE_OK, r);
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_open_memory(a, buff, sizeof(buff), &used));
		/* Write a file to it. */
		assert((ae = archive_entry_new()) != NULL);
		archive_entry_copy_pathname(ae, "file");
		archive_entry_set_mode(ae, S_IFREG | 0755);
		archive_entry_set_size(ae, 0);
		assertA(0 == archive_write_header(a, ae));
		archive_entry_free(ae);

		/* THE TEST: write zero bytes to this entry. */
		assertEqualIntA(a, 0, archive_write_data(a, "", 0));

		/* Close out the archive. */
		assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
		assertEqualInt(ARCHIVE_OK, archive_write_free(a));
	}

	/*
	 * For good measure, one more time with no compression.
	 */

	/* Create a new archive in memory. */
	assert((a = archive_write_new()) != NULL);
	assertA(0 == archive_write_set_format_ustar(a));
	assertA(0 == archive_write_add_filter_none(a));
	assertA(0 == archive_write_open_memory(a, buff, sizeof(buff), &used));
	/* Write a file to it. */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file");
	archive_entry_set_mode(ae, S_IFREG | 0755);
	archive_entry_set_size(ae, 0);
	assertA(0 == archive_write_header(a, ae));
	archive_entry_free(ae);

	/* THE TEST: write zero bytes to this entry. */
	assertEqualIntA(a, 0, archive_write_data(a, "", 0));

	/* Close out the archive. */
	assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
}