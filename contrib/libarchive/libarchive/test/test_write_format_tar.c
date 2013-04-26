
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

char buff[1000000];
char buff2[64];

DEFINE_TEST(test_write_format_tar)
{
	struct archive_entry *ae;
	struct archive *a;
	char *p;
	size_t used;
	size_t blocksize;

	/* Repeat the following for a variety of odd blocksizes. */
	for (blocksize = 1; blocksize < 100000; blocksize += blocksize + 3) {
		/* Create a new archive in memory. */
		assert((a = archive_write_new()) != NULL);
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_set_format_ustar(a));
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_add_filter_none(a));
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_set_bytes_per_block(a, (int)blocksize));
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_set_bytes_in_last_block(a, (int)blocksize));
		assertEqualInt(blocksize,
		    archive_write_get_bytes_in_last_block(a));
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_open_memory(a, buff, sizeof(buff), &used));
		assertEqualInt(blocksize,
		    archive_write_get_bytes_in_last_block(a));

		/*
		 * Write a file to it.
		 */
		assert((ae = archive_entry_new()) != NULL);
		archive_entry_set_mtime(ae, 1, 10);
		assertEqualInt(1, archive_entry_mtime(ae));
		assertEqualInt(10, archive_entry_mtime_nsec(ae));
		p = strdup("file");
		archive_entry_copy_pathname(ae, p);
		strcpy(p, "XXXX");
		free(p);
		assertEqualString("file", archive_entry_pathname(ae));
		archive_entry_set_mode(ae, S_IFREG | 0755);
		assertEqualInt(S_IFREG | 0755, archive_entry_mode(ae));
		archive_entry_set_size(ae, 8);

		assertEqualIntA(a, ARCHIVE_OK, archive_write_header(a, ae));
		archive_entry_free(ae);
		assertEqualInt(8, archive_write_data(a, "12345678", 9));

		/* Close out the archive. */
		assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
		assertEqualInt(ARCHIVE_OK, archive_write_free(a));

		/* This calculation gives "the smallest multiple of
		 * the block size that is at least 2048 bytes". */
		failure("blocksize=%d", blocksize);
		assertEqualInt(((2048 - 1)/blocksize+1)*blocksize, used);

		/*
		 * Now, read the data back.
		 */
		assert((a = archive_read_new()) != NULL);
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_read_support_format_all(a));
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_read_support_filter_all(a));
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_read_open_memory(a, buff, used));

		assertEqualIntA(a, ARCHIVE_OK,
		    archive_read_next_header(a, &ae));

		assertEqualInt(1, archive_entry_mtime(ae));
		/* Not the same as above: ustar doesn't store hi-res times. */
		assertEqualInt(0, archive_entry_mtime_nsec(ae));
		assertEqualInt(0, archive_entry_atime(ae));
		assertEqualInt(0, archive_entry_ctime(ae));
		assertEqualString("file", archive_entry_pathname(ae));
		assertEqualInt(AE_IFREG, archive_entry_filetype(ae));
		assertEqualInt(AE_IFREG | 0755, archive_entry_mode(ae));
		assertEqualInt(8, archive_entry_size(ae));
		assertEqualInt(8, archive_read_data(a, buff2, 10));
		assertEqualMem(buff2, "12345678", 8);

		/* Verify the end of the archive. */
		assertEqualIntA(a, ARCHIVE_EOF,
		    archive_read_next_header(a, &ae));
		assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
		assertEqualInt(ARCHIVE_OK, archive_read_free(a));
	}
}