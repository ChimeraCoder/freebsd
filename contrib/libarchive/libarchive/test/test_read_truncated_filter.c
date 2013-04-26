
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
 * Check that we generate an error message when reading a truncated
 * gzip, bzip2, compress, xz, lzma, or lzip file.
 */

static void
test_truncation(const char *compression,
    int (*set_compression)(struct archive *), int can_prog)
{
	struct archive_entry *ae;
	struct archive* a;
	char path[16];
	char *buff, *data;
	size_t buffsize, datasize, used1;
	int i, j, r, use_prog;

	buffsize = 2000000;
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

	/*
	 * Write a bunch of files with semi-random data.
	 */
	assert((a = archive_write_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_write_set_format_ustar(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_add_filter_compress(a));
	r = (*set_compression)(a);
	if (r != ARCHIVE_OK && !can_prog) {
		skipping("%s writing not supported on this platform",
		    compression);
		assertEqualInt(ARCHIVE_OK, archive_write_free(a));
		free(buff);
		free(data);
		return;
	}
	use_prog = (r == ARCHIVE_WARN && can_prog);
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_set_bytes_per_block(a, 10));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_open_memory(a, buff, buffsize, &used1));
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_set_filetype(ae, AE_IFREG);
	archive_entry_set_size(ae, datasize);
	for (i = 0; i < 100; i++) {
		sprintf(path, "%s%d", compression, i);
		archive_entry_copy_pathname(ae, path);
		failure(path);
		if (!assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_header(a, ae))) {
			archive_write_free(a);
			free(data);
			free(buff);
			return;
		}
		for (j = 0; j < (int)datasize; ++j) {
			data[j] = (char)(rand() % 256);
		}
		failure(path);
		if (!assertEqualIntA(a, datasize,
		    archive_write_data(a, data, datasize))) {
			archive_write_free(a);
			free(data);
			free(buff);
			return;
		}
	}
	archive_entry_free(ae);
	assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));

	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));

	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_open_memory(a, buff, used1 - used1/64));
	for (i = 0; i < 100; i++) {
		if (ARCHIVE_OK != archive_read_next_header(a, &ae)) {
			failure("Should have non-NULL error message for %s",
			    compression);
			assert(NULL != archive_error_string(a));
			break;
		}
		sprintf(path, "%s%d", compression, i);
		assertEqualString(path, archive_entry_pathname(ae));
		if (datasize != (size_t)archive_read_data(a, data, datasize)) {
			failure("Should have non-NULL error message for %s",
			    compression);
			assert(NULL != archive_error_string(a));
			break;
		}
	}
	assertEqualIntA(a, (use_prog)?ARCHIVE_WARN:ARCHIVE_OK,
	    archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));

	free(data);
	free(buff);
}

DEFINE_TEST(test_read_truncated_filter_bzip2)
{
	test_truncation("bzip2", archive_write_add_filter_bzip2, canBzip2());
}

DEFINE_TEST(test_read_truncated_filter_compress)
{
	test_truncation("compress", archive_write_add_filter_compress, 0);
}

DEFINE_TEST(test_read_truncated_filter_gzip)
{
	test_truncation("gzip", archive_write_add_filter_gzip, canGzip());
}

DEFINE_TEST(test_read_truncated_filter_lzip)
{
	test_truncation("lzip", archive_write_add_filter_lzip, 0);
}

DEFINE_TEST(test_read_truncated_filter_lzma)
{
	test_truncation("lzma", archive_write_add_filter_lzma, 0);
}

DEFINE_TEST(test_read_truncated_filter_lzop)
{
	test_truncation("lzop", archive_write_add_filter_lzop, canLzop());
}

DEFINE_TEST(test_read_truncated_filter_xz)
{
	test_truncation("xz", archive_write_add_filter_xz, 0);
}