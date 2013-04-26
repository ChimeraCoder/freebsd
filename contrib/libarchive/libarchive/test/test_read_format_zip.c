
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

#ifdef HAVE_LIBZ
static const int libz_enabled = 1;
#else
static const int libz_enabled = 0;
#endif

/*
 * The reference file for this has been manually tweaked so that:
 *   * file2 has length-at-end but file1 does not
 *   * file2 has an invalid CRC
 */
static void
verify_basic(struct archive *a, int seek_checks)
{
	struct archive_entry *ae;
	char *buff[128];
	const void *pv;
	size_t s;
	int64_t o;

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("dir/", archive_entry_pathname(ae));
	assertEqualInt(1179604249, archive_entry_mtime(ae));
	assertEqualInt(0, archive_entry_size(ae));
	if (seek_checks)
		assertEqualInt(AE_IFDIR | 0755, archive_entry_mode(ae));
	assertEqualIntA(a, ARCHIVE_EOF,
	    archive_read_data_block(a, &pv, &s, &o));
	assertEqualInt((int)s, 0);

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("file1", archive_entry_pathname(ae));
	assertEqualInt(1179604289, archive_entry_mtime(ae));
	if (seek_checks)
		assertEqualInt(AE_IFREG | 0755, archive_entry_mode(ae));
	assertEqualInt(18, archive_entry_size(ae));
	failure("archive_read_data() returns number of bytes read");
	if (libz_enabled) {
		assertEqualInt(18, archive_read_data(a, buff, 19));
		assertEqualMem(buff, "hello\nhello\nhello\n", 18);
	} else {
		assertEqualInt(ARCHIVE_FAILED, archive_read_data(a, buff, 19));
		assertEqualString(archive_error_string(a),
		    "Unsupported ZIP compression method (deflation)");
		assert(archive_errno(a) != 0);
	}

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("file2", archive_entry_pathname(ae));
	assertEqualInt(1179605932, archive_entry_mtime(ae));
	if (seek_checks) {
		assertEqualInt(AE_IFREG | 0755, archive_entry_mode(ae));
		assertEqualInt(64, archive_entry_size_is_set(ae));
	} else {
		failure("file2 has length-at-end, so we shouldn't see a valid size when streaming");
		assertEqualInt(0, archive_entry_size_is_set(ae));
	}
	if (libz_enabled) {
		failure("file2 has a bad CRC, so read should fail and not change buff");
		memset(buff, 'a', 19);
		assertEqualInt(ARCHIVE_WARN, archive_read_data(a, buff, 19));
		assertEqualMem(buff, "aaaaaaaaaaaaaaaaaaa", 19);
	} else {
		assertEqualInt(ARCHIVE_FAILED, archive_read_data(a, buff, 19));
		assertEqualString(archive_error_string(a),
		    "Unsupported ZIP compression method (deflation)");
		assert(archive_errno(a) != 0);
	}
	assertEqualInt(ARCHIVE_EOF, archive_read_next_header(a, &ae));
	/* Verify the number of files read. */
	failure("the archive file has three files");
	assertEqualInt(3, archive_file_count(a));
	assertEqualIntA(a, ARCHIVE_FILTER_NONE, archive_filter_code(a, 0));
	assertEqualIntA(a, ARCHIVE_FORMAT_ZIP, archive_format(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

static void
test_basic(void)
{
	const char *refname = "test_read_format_zip.zip";
	struct archive *a;
	char *p;
	size_t s;

	extract_reference_file(refname);

	/* Verify with seeking reader. */
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_filename(a, refname, 10240));
	verify_basic(a, 1);

	/* Verify with streaming reader. */
	p = slurpfile(&s, refname);
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, read_open_memory(a, p, s, 31));
	verify_basic(a, 0);
}

/*
 * Read Info-ZIP New Unix Extra Field 0x7875 "ux".
 *  Currently stores Unix UID/GID up to 32 bits.
 */
static void
verify_info_zip_ux(struct archive *a, int seek_checks)
{
	struct archive_entry *ae;
	char *buff[128];

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("file1", archive_entry_pathname(ae));
	assertEqualInt(1300668680, archive_entry_mtime(ae));
	assertEqualInt(18, archive_entry_size(ae));
	if (seek_checks)
		assertEqualInt(AE_IFREG | 0644, archive_entry_mode(ae));
	failure("zip reader should read Info-ZIP New Unix Extra Field");
	assertEqualInt(1001, archive_entry_uid(ae));
	assertEqualInt(1001, archive_entry_gid(ae));
	if (libz_enabled) {
		failure("archive_read_data() returns number of bytes read");
		assertEqualInt(18, archive_read_data(a, buff, 19));
		assertEqualMem(buff, "hello\nhello\nhello\n", 18);
	} else {
		assertEqualInt(ARCHIVE_FAILED, archive_read_data(a, buff, 19));
		assertEqualString(archive_error_string(a),
		    "Unsupported ZIP compression method (deflation)");
		assert(archive_errno(a) != 0);
	}
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify the number of files read. */
	failure("the archive file has just one file");
	assertEqualInt(1, archive_file_count(a));

	assertEqualIntA(a, ARCHIVE_FILTER_NONE, archive_filter_code(a, 0));
	assertEqualIntA(a, ARCHIVE_FORMAT_ZIP, archive_format(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

static void
test_info_zip_ux(void)
{
	const char *refname = "test_read_format_zip_ux.zip";
	struct archive *a;
	char *p;
	size_t s;

	extract_reference_file(refname);

	/* Verify with seeking reader. */
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_filename(a, refname, 10240));
	verify_info_zip_ux(a, 1);

	/* Verify with streaming reader. */
	p = slurpfile(&s, refname);
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, read_open_memory(a, p, s, 108));
	verify_info_zip_ux(a, 0);
}

/*
 * Verify that test_read_extract correctly works with
 * Zip entries that use length-at-end.
 */
static void
verify_extract_length_at_end(struct archive *a, int seek_checks)
{
	struct archive_entry *ae;

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));

	assertEqualString("hello.txt", archive_entry_pathname(ae));
	if (seek_checks) {
		assertEqualInt(AE_IFREG | 0644, archive_entry_mode(ae));
		assert(archive_entry_size_is_set(ae));
		assertEqualInt(6, archive_entry_size(ae));
	} else {
		assert(!archive_entry_size_is_set(ae));
		assertEqualInt(0, archive_entry_size(ae));
	}

	if (libz_enabled) {
		assertEqualIntA(a, ARCHIVE_OK, archive_read_extract(a, ae, 0));
		assertFileContents("hello\x0A", 6, "hello.txt");
	} else {
		assertEqualIntA(a, ARCHIVE_FAILED, archive_read_extract(a, ae, 0));
		assertEqualString(archive_error_string(a),
		    "Unsupported ZIP compression method (deflation)");
		assert(archive_errno(a) != 0);
	}

	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_free(a));
}

static void
test_extract_length_at_end(void)
{
	const char *refname = "test_read_format_zip_length_at_end.zip";
	char *p;
	size_t s;
	struct archive *a;

	extract_reference_file(refname);

	/* Verify extraction with seeking reader. */
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_filename(a, refname, 10240));
	verify_extract_length_at_end(a, 1);

	/* Verify extraction with streaming reader. */
	p = slurpfile(&s, refname);
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, read_open_memory(a, p, s, 108));
	verify_extract_length_at_end(a, 0);
}

static void
test_symlink(void)
{
	const char *refname = "test_read_format_zip_symlink.zip";
	char *p;
	size_t s;
	struct archive *a;
	struct archive_entry *ae;

	extract_reference_file(refname);
	p = slurpfile(&s, refname);

	/* Symlinks can only be extracted with the seeking reader. */
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_zip(a));
	assertEqualIntA(a, ARCHIVE_OK, read_open_memory_seek(a, p, s, 1));

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("file", archive_entry_pathname(ae));
	assertEqualInt(AE_IFREG, archive_entry_filetype(ae));

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("symlink", archive_entry_pathname(ae));
	assertEqualInt(AE_IFLNK, archive_entry_filetype(ae));
	assertEqualInt(0, archive_entry_size(ae));
	assertEqualString("file", archive_entry_symlink(ae));

	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_free(a));
}

DEFINE_TEST(test_read_format_zip)
{
	test_basic();
	test_info_zip_ux();
	test_extract_length_at_end();
	test_symlink();
}