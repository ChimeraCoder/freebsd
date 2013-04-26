
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
 * Verify our ability to read sample files created by Solaris pax for
 * a sparse file.
 */
static void
test_compat_solaris_pax_sparse_1(void)
{
	char name[] = "test_compat_solaris_pax_sparse_1.pax.Z";
	struct archive_entry *ae;
	struct archive *a;
	int64_t offset, length;
	const void *buff;
	size_t bytes_read;
	char data[1024*8];
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
	assertEqualString("hole", archive_entry_pathname(ae));
	assertEqualInt(1310411683, archive_entry_mtime(ae));
	assertEqualInt(101, archive_entry_uid(ae));
	assertEqualString("cue", archive_entry_uname(ae));
	assertEqualInt(10, archive_entry_gid(ae));
	assertEqualString("staff", archive_entry_gname(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	/* Verify the sparse information. */
	failure("This sparse file should have tree data blocks");
	assertEqualInt(3, archive_entry_sparse_reset(ae));
	assertEqualInt(ARCHIVE_OK,
	    archive_entry_sparse_next(ae, &offset, &length));
	assertEqualInt(0, offset);
	assertEqualInt(131072, length);
	assertEqualInt(ARCHIVE_OK,
	    archive_entry_sparse_next(ae, &offset, &length));
	assertEqualInt(393216, offset);
	assertEqualInt(131072, length);
	assertEqualInt(ARCHIVE_OK,
	    archive_entry_sparse_next(ae, &offset, &length));
	assertEqualInt(786432, offset);
	assertEqualInt(32775, length);
	while (ARCHIVE_OK ==
	    archive_read_data_block(a, &buff, &bytes_read, &offset)) {
		failure("The data blocks should not include the hole");
		assert((offset >= 0 && offset + bytes_read <= 131072) ||
		       (offset >= 393216 && offset + bytes_read <= 393216+131072) ||
		       (offset >= 786432 && offset + bytes_read <= 786432+32775));
		if (offset == 0 && bytes_read >= 1024*8) {
			memset(data, 'a', sizeof(data));
			failure("First data block should be 8K bytes of 'a'");
			assertEqualMem(buff, data, sizeof(data));
		} else if (offset + bytes_read == 819207 && bytes_read >= 7) {
			const char *last = buff;
			last += bytes_read - 7;
			memset(data, 'c', 7);
			failure("Last seven bytes should be all 'c'");
			assertEqualMem(last, data, 7);
		}
	}

	/* Verify the end-of-archive. */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify that the format detection worked. */
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_COMPRESS);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE);

	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

/*
 * Verify our ability to read sample files created by Solaris pax for
 * a sparse file which begin with hole.
 */
static void
test_compat_solaris_pax_sparse_2(void)
{
	char name[] = "test_compat_solaris_pax_sparse_2.pax.Z";
	struct archive_entry *ae;
	struct archive *a;
	int64_t offset, length;
	const void *buff;
	size_t bytes_read;
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
	assertEqualString("hole", archive_entry_pathname(ae));
	assertEqualInt(1310416789, archive_entry_mtime(ae));
	assertEqualInt(101, archive_entry_uid(ae));
	assertEqualString("cue", archive_entry_uname(ae));
	assertEqualInt(10, archive_entry_gid(ae));
	assertEqualString("staff", archive_entry_gname(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	/* Verify the sparse information. */
	failure("This sparse file should have two data blocks");
	assertEqualInt(2, archive_entry_sparse_reset(ae));
	assertEqualInt(ARCHIVE_OK,
	    archive_entry_sparse_next(ae, &offset, &length));
	assertEqualInt(393216, offset);
	assertEqualInt(131072, length);
	assertEqualInt(ARCHIVE_OK,
	    archive_entry_sparse_next(ae, &offset, &length));
	assertEqualInt(786432, offset);
	assertEqualInt(32799, length);
	while (ARCHIVE_OK ==
	    archive_read_data_block(a, &buff, &bytes_read, &offset)) {
		failure("The data blocks should not include the hole");
		assert((offset >= 393216 && offset + bytes_read <= 393216+131072) ||
		       (offset >= 786432 && offset + bytes_read <= 786432+32799));
		if (offset + bytes_read == 819231 && bytes_read >= 31) {
			char data[32];
			const char *last = buff;
			last += bytes_read - 31;
			memset(data, 'c', 31);
			failure("Last 31 bytes should be all 'c'");
			assertEqualMem(last, data, 31);
		}
	}

	/* Verify the end-of-archive. */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify that the format detection worked. */
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_COMPRESS);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE);

	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}


DEFINE_TEST(test_compat_solaris_pax_sparse)
{
	test_compat_solaris_pax_sparse_1();
	test_compat_solaris_pax_sparse_2();
}