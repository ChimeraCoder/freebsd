
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

DEFINE_TEST(test_write_filter_gzip_timestamp)
{
	struct archive_entry *ae;
	struct archive* a;
	char *buff, *data;
	size_t buffsize, datasize;
	size_t used1;
	int r, use_prog = 0;

	buffsize = 10000;
	assert(NULL != (buff = (char *)malloc(buffsize)));
	if (buff == NULL)
		return;

	datasize = 10000;
	assert(NULL != (data = (char *)malloc(datasize)));
	if (data == NULL) {
		free(buff);
		return;
	}
	memset(data, 0, datasize);

	/* Test1: set "gzip:timestamp" option. */
	assert((a = archive_write_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_write_set_format_ustar(a));
	r = archive_write_add_filter_gzip(a);
	if (r != ARCHIVE_OK) {
		if (canGzip() && r == ARCHIVE_WARN)
			use_prog = 1;
		else {
			skipping("gzip writing not supported on this platform");
			assertEqualInt(ARCHIVE_OK, archive_write_free(a));
			free(buff);
			free(data);
			return;
		}
	}
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_set_options(a, "gzip:timestamp"));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_set_bytes_per_block(a, 10));
	assertEqualInt(ARCHIVE_FILTER_GZIP, archive_filter_code(a, 0));
	assertEqualString("gzip", archive_filter_name(a, 0));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_open_memory(a, buff, buffsize, &used1));
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_set_filetype(ae, AE_IFREG);
	archive_entry_set_size(ae, datasize);
	archive_entry_copy_pathname(ae, "file");
	assertEqualIntA(a, ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualIntA(a, datasize, archive_write_data(a, data, datasize));
	archive_entry_free(ae);
	assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
	failure("Timestamp should be recorded");
	assert(memcmp(buff + 4, "\x00\x00\x00\x00", 4) != 0);

	/* Test2: set "gzip:!timestamp" option. */
	assert((a = archive_write_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_write_set_format_ustar(a));
	assertEqualIntA(a, (use_prog)?ARCHIVE_WARN:ARCHIVE_OK,
	    archive_write_add_filter_gzip(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_set_options(a, "gzip:!timestamp"));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_set_bytes_per_block(a, 10));
	assertEqualInt(ARCHIVE_FILTER_GZIP, archive_filter_code(a, 0));
	assertEqualString("gzip", archive_filter_name(a, 0));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_open_memory(a, buff, buffsize, &used1));
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_set_filetype(ae, AE_IFREG);
	archive_entry_set_size(ae, datasize);
	archive_entry_copy_pathname(ae, "file");
	assertEqualIntA(a, ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualIntA(a, datasize, archive_write_data(a, data, datasize));
	archive_entry_free(ae);
	assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
	failure("Timestamp should not be recorded");
	assertEqualMem(buff + 4, "\x00\x00\x00\x00", 4);

	/*
	 * Clean up.
	 */
	free(data);
	free(buff);
}