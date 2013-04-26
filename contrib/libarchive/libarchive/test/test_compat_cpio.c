
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
 * Verify our ability to read various sample files.
 * It should be easy to add any new sample files sent in by users
 * to this collection of tests.
 */

/* Copy this function for each test file and adjust it accordingly. */

/*
 * test_compat_cpio_1.cpio checks heuristics for avoiding false
 * hardlinks.  foo1 and foo2 are files that have nlinks=1 and so
 * should not be marked as hardlinks even though they have identical
 * ino values.  bar1 and bar2 have nlinks=2 so should be marked
 * as hardlinks.
 */
static void
test_compat_cpio_1(void)
{
	char name[] = "test_compat_cpio_1.cpio";
	struct archive_entry *ae;
	struct archive *a;

	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	extract_reference_file(name);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_filename(a, name, 17));

	/* Read first entry. */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("foo1", archive_entry_pathname(ae));
	assertEqualString(NULL, archive_entry_hardlink(ae));
	assertEqualInt(1260250228, archive_entry_mtime(ae));
	assertEqualInt(1000, archive_entry_uid(ae));
	assertEqualInt(1000, archive_entry_gid(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	/* Read second entry. */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("foo2", archive_entry_pathname(ae));
	assertEqualString(NULL, archive_entry_hardlink(ae));
	assertEqualInt(1260250228, archive_entry_mtime(ae));
	assertEqualInt(1000, archive_entry_uid(ae));
	assertEqualInt(1000, archive_entry_gid(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	/* Read third entry. */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("bar1", archive_entry_pathname(ae));
	assertEqualString(NULL, archive_entry_hardlink(ae));
	assertEqualInt(1260250228, archive_entry_mtime(ae));
	assertEqualInt(1000, archive_entry_uid(ae));
	assertEqualInt(1000, archive_entry_gid(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	/* Read fourth entry. */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("bar2", archive_entry_pathname(ae));
	assertEqualString("bar1", archive_entry_hardlink(ae));
	assertEqualInt(1260250228, archive_entry_mtime(ae));
	assertEqualInt(1000, archive_entry_uid(ae));
	assertEqualInt(1000, archive_entry_gid(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	/* Verify that the format detection worked. */
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_NONE);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_CPIO_SVR4_NOCRC);

	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}


DEFINE_TEST(test_compat_cpio)
{
	test_compat_cpio_1();
}