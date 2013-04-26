
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
__FBSDID("$FreeBSD: head/lib/libarchive/test/test_read_format_isojoliet_bz2.c 201247 2009-12-30 05:59:21Z kientzle $");

/*
 * The data for this testcase was provided by Mike Qin <mikeandmore@gmail.com>
 * and created with Nero.
 */

DEFINE_TEST(test_read_format_isojoliet_versioned)
{
	const char *refname = "test_read_format_iso_joliet_by_nero.iso.Z";
	struct archive_entry *ae;
	struct archive *a;

	extract_reference_file(refname);
	assert((a = archive_read_new()) != NULL);
	assertEqualInt(0, archive_read_support_filter_all(a));
	assertEqualInt(0, archive_read_support_format_all(a));
	assertEqualInt(ARCHIVE_OK,
	    archive_read_set_option(a, "iso9660", "rockridge", NULL));
	assertEqualInt(ARCHIVE_OK,
	    archive_read_open_filename(a, refname, 10240));

	/* First entry is '.' root directory. */
	assertEqualInt(0, archive_read_next_header(a, &ae));
	assertEqualString(".", archive_entry_pathname(ae));
	assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));

	/* A directory. */
	assertEqualInt(0, archive_read_next_header(a, &ae));
	assertEqualString("test", archive_entry_pathname(ae));
	assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));

	/* A regular file which is called test.txt and has
	 * ;1 appended to it because apparently Nero always
	 * appends versions to all files in the joliet extension.
	 *
	 * We test to make sure the version has been stripped.
	 */
	assertEqualInt(0, archive_read_next_header(a, &ae));
	assertEqualString("test/test.txt",
	    archive_entry_pathname(ae));
	assertEqualInt(AE_IFREG, archive_entry_filetype(ae));

	/* End of archive. */
	assertEqualInt(ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify archive format. */
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_COMPRESS);

	/* Close the archive. */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}