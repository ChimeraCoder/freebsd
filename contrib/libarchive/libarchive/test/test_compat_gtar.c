
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
 * Verify our ability to read sample files created by GNU tar.
 * It should be easy to add any new sample files sent in by users
 * to this collection of tests.
 */

/* Copy this function for each test file and adjust it accordingly. */

/*
 * test_compat_gtar_1.tgz exercises reading long filenames and
 * symlink targets stored in the GNU tar format.
 */
static void
test_compat_gtar_1(void)
{
	char name[] = "test_compat_gtar_1.tar";
	struct archive_entry *ae;
	struct archive *a;
	int r;

	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	extract_reference_file(name);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_filename(a, name, 10240));

	/* Read first entry. */
	assertEqualIntA(a, ARCHIVE_OK, r = archive_read_next_header(a, &ae));
	if (r != ARCHIVE_OK) {
		archive_read_free(a);
		return;
	}
	assertEqualString(
		"12345678901234567890123456789012345678901234567890"
		"12345678901234567890123456789012345678901234567890"
		"12345678901234567890123456789012345678901234567890"
		"12345678901234567890123456789012345678901234567890",
		archive_entry_pathname(ae));
	assertEqualInt(1197179003, archive_entry_mtime(ae));
	assertEqualInt(1000, archive_entry_uid(ae));
	assertEqualString("tim", archive_entry_uname(ae));
	assertEqualInt(1000, archive_entry_gid(ae));
	assertEqualString("tim", archive_entry_gname(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	/* Read second entry. */
	assertEqualIntA(a, ARCHIVE_OK, r = archive_read_next_header(a, &ae));
	if (r != ARCHIVE_OK) {
		archive_read_free(a);
		return;
	}
	assertEqualString(
		"abcdefghijabcdefghijabcdefghijabcdefghijabcdefghij"
		"abcdefghijabcdefghijabcdefghijabcdefghijabcdefghij"
		"abcdefghijabcdefghijabcdefghijabcdefghijabcdefghij"
		"abcdefghijabcdefghijabcdefghijabcdefghijabcdefghij",
		archive_entry_pathname(ae));
	assertEqualString(
		"12345678901234567890123456789012345678901234567890"
		"12345678901234567890123456789012345678901234567890"
		"12345678901234567890123456789012345678901234567890"
		"12345678901234567890123456789012345678901234567890",
		archive_entry_symlink(ae));
	assertEqualInt(1197179043, archive_entry_mtime(ae));
	assertEqualInt(1000, archive_entry_uid(ae));
	assertEqualString("tim", archive_entry_uname(ae));
	assertEqualInt(1000, archive_entry_gid(ae));
	assertEqualString("tim", archive_entry_gname(ae));
	assertEqualInt(0120755, archive_entry_mode(ae));

	/* Verify the end-of-archive. */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify that the format detection worked. */
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_NONE);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_GNUTAR);

	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}


DEFINE_TEST(test_compat_gtar)
{
	test_compat_gtar_1();
}