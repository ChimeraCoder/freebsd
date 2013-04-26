
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
 * Verify our ability to read sample files compatibly with bunzip2.
 *
 * In particular:
 *  * bunzip2 will read multiple bzip2 streams, concatenating the output
 *  * bunzip2 will stop at the end of a stream if the following data
 *    doesn't start with a bzip2 signature.
 */

/*
 * All of the sample files have the same contents; they're just
 * compressed in different ways.
 */
static void
compat_bzip2(const char *name)
{
	const char *n[7] = { "f1", "f2", "f3", "d1/f1", "d1/f2", "d1/f3", NULL };
	struct archive_entry *ae;
	struct archive *a;
	int i;

	assert((a = archive_read_new()) != NULL);
	if (ARCHIVE_OK != archive_read_support_filter_bzip2(a)) {
		skipping("Unsupported bzip2");
		return;
	}
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	extract_reference_file(name);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_filename(a, name, 2));

	/* Read entries, match up names with list above. */
	for (i = 0; i < 6; ++i) {
		failure("Could not read file %d (%s) from %s", i, n[i], name);
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_read_next_header(a, &ae));
		assertEqualString(n[i], archive_entry_pathname(ae));
	}

	/* Verify the end-of-archive. */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify that the format detection worked. */
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_BZIP2);
	assertEqualString(archive_filter_name(a, 0), "bzip2");
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_USTAR);

	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_BZIP2);
	assertEqualString(archive_filter_name(a, 0), "bzip2");
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_USTAR);

	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}


DEFINE_TEST(test_compat_bzip2)
{
	compat_bzip2("test_compat_bzip2_1.tbz");
	compat_bzip2("test_compat_bzip2_2.tbz");
}