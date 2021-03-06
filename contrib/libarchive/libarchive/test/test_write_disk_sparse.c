
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
 * Write a file using archive_write_data call, read the file
 * back and verify the contents.  The data written includes large
 * blocks of nulls, so it should exercise the sparsification logic
 * if ARCHIVE_EXTRACT_SPARSE is enabled.
 */
static void
verify_write_data(struct archive *a, int sparse)
{
	static const char data[]="abcdefghijklmnopqrstuvwxyz";
	struct stat st;
	struct archive_entry *ae;
	size_t buff_size = 64 * 1024;
	char *buff, *p;
	const char *msg = sparse ? "sparse" : "non-sparse";
	FILE *f;

	buff = malloc(buff_size);
	assert(buff != NULL);
	if (buff == NULL)
		return;

	ae = archive_entry_new();
	assert(ae != NULL);
	archive_entry_set_size(ae, 8 * buff_size);
	archive_entry_set_pathname(ae, "test_write_data");
	archive_entry_set_mode(ae, AE_IFREG | 0755);
	assertEqualIntA(a, 0, archive_write_header(a, ae));

	/* Use archive_write_data() to write three relatively sparse blocks. */

	/* First has non-null data at beginning. */
	memset(buff, 0, buff_size);
	memcpy(buff, data, sizeof(data));
	failure("%s", msg);
	assertEqualInt(buff_size, archive_write_data(a, buff, buff_size));

	/* Second has non-null data in the middle. */
	memset(buff, 0, buff_size);
	memcpy(buff + buff_size / 2 - 3, data, sizeof(data));
	failure("%s", msg);
	assertEqualInt(buff_size, archive_write_data(a, buff, buff_size));

	/* Third has non-null data at the end. */
	memset(buff, 0, buff_size);
	memcpy(buff + buff_size - sizeof(data), data, sizeof(data));
	failure("%s", msg);
	assertEqualInt(buff_size, archive_write_data(a, buff, buff_size));

	failure("%s", msg);
	assertEqualIntA(a, 0, archive_write_finish_entry(a));

	/* Test the entry on disk. */
	assert(0 == stat(archive_entry_pathname(ae), &st));
        assertEqualInt(st.st_size, 8 * buff_size);
	f = fopen(archive_entry_pathname(ae), "rb");
	assert(f != NULL);
	if (f == NULL) {
		free(buff);
		return;
	}

	/* Check first block. */
	assertEqualInt(buff_size, fread(buff, 1, buff_size, f));
	failure("%s", msg);
	assertEqualMem(buff, data, sizeof(data));
	for (p = buff + sizeof(data); p < buff + buff_size; ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (!assertEqualInt(0, *p))
			break;
	}

	/* Check second block. */
	assertEqualInt(buff_size, fread(buff, 1, buff_size, f));
	for (p = buff; p < buff + buff_size; ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (p == buff + buff_size / 2 - 3) {
			assertEqualMem(p, data, sizeof(data));
			p += sizeof(data);
		} else if (!assertEqualInt(0, *p))
			break;
	}

	/* Check third block. */
	assertEqualInt(buff_size, fread(buff, 1, buff_size, f));
	for (p = buff; p < buff + buff_size - sizeof(data); ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (!assertEqualInt(0, *p))
			break;
	}
	failure("%s", msg);
	assertEqualMem(buff + buff_size - sizeof(data), data, sizeof(data));

	/* XXX more XXX */

	assertEqualInt(0, fclose(f));
	archive_entry_free(ae);
	free(buff);
}

/*
 * As above, but using the archive_write_data_block() call.
 */
static void
verify_write_data_block(struct archive *a, int sparse)
{
	static const char data[]="abcdefghijklmnopqrstuvwxyz";
	struct stat st;
	struct archive_entry *ae;
	size_t buff_size = 64 * 1024;
	char *buff, *p;
	const char *msg = sparse ? "sparse" : "non-sparse";
	FILE *f;

	buff = malloc(buff_size);
	assert(buff != NULL);
	if (buff == NULL)
		return;

	ae = archive_entry_new();
	assert(ae != NULL);
	archive_entry_set_size(ae, 8 * buff_size);
	archive_entry_set_pathname(ae, "test_write_data_block");
	archive_entry_set_mode(ae, AE_IFREG | 0755);
	assertEqualIntA(a, 0, archive_write_header(a, ae));

	/* Use archive_write_data_block() to write three
	   relatively sparse blocks. */

	/* First has non-null data at beginning. */
	memset(buff, 0, buff_size);
	memcpy(buff, data, sizeof(data));
	failure("%s", msg);
	assertEqualInt(ARCHIVE_OK,
	    archive_write_data_block(a, buff, buff_size, 100));

	/* Second has non-null data in the middle. */
	memset(buff, 0, buff_size);
	memcpy(buff + buff_size / 2 - 3, data, sizeof(data));
	failure("%s", msg);
	assertEqualInt(ARCHIVE_OK,
	    archive_write_data_block(a, buff, buff_size, buff_size + 200));

	/* Third has non-null data at the end. */
	memset(buff, 0, buff_size);
	memcpy(buff + buff_size - sizeof(data), data, sizeof(data));
	failure("%s", msg);
	assertEqualInt(ARCHIVE_OK,
	    archive_write_data_block(a, buff, buff_size, buff_size * 2 + 300));

	failure("%s", msg);
	assertEqualIntA(a, 0, archive_write_finish_entry(a));

	/* Test the entry on disk. */
	assert(0 == stat(archive_entry_pathname(ae), &st));
        assertEqualInt(st.st_size, 8 * buff_size);
	f = fopen(archive_entry_pathname(ae), "rb");
	assert(f != NULL);
	if (f == NULL) {
		free(buff);
		return;
	}

	/* Check 100-byte gap at beginning */
	assertEqualInt(100, fread(buff, 1, 100, f));
	failure("%s", msg);
	for (p = buff; p < buff + 100; ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (!assertEqualInt(0, *p))
			break;
	}

	/* Check first block. */
	assertEqualInt(buff_size, fread(buff, 1, buff_size, f));
	failure("%s", msg);
	assertEqualMem(buff, data, sizeof(data));
	for (p = buff + sizeof(data); p < buff + buff_size; ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (!assertEqualInt(0, *p))
			break;
	}

	/* Check 100-byte gap */
	assertEqualInt(100, fread(buff, 1, 100, f));
	failure("%s", msg);
	for (p = buff; p < buff + 100; ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (!assertEqualInt(0, *p))
			break;
	}

	/* Check second block. */
	assertEqualInt(buff_size, fread(buff, 1, buff_size, f));
	for (p = buff; p < buff + buff_size; ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (p == buff + buff_size / 2 - 3) {
			assertEqualMem(p, data, sizeof(data));
			p += sizeof(data);
		} else if (!assertEqualInt(0, *p))
			break;
	}

	/* Check 100-byte gap */
	assertEqualInt(100, fread(buff, 1, 100, f));
	failure("%s", msg);
	for (p = buff; p < buff + 100; ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (!assertEqualInt(0, *p))
			break;
	}

	/* Check third block. */
	assertEqualInt(buff_size, fread(buff, 1, buff_size, f));
	for (p = buff; p < buff + buff_size - sizeof(data); ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (!assertEqualInt(0, *p))
			break;
	}
	failure("%s", msg);
	assertEqualMem(buff + buff_size - sizeof(data), data, sizeof(data));

	/* Check another block size beyond last we wrote. */
	assertEqualInt(buff_size, fread(buff, 1, buff_size, f));
	failure("%s", msg);
	for (p = buff; p < buff + buff_size; ++p) {
		failure("offset: %d, %s", (int)(p - buff), msg);
		if (!assertEqualInt(0, *p))
			break;
	}


	/* XXX more XXX */

	assertEqualInt(0, fclose(f));
	free(buff);
	archive_entry_free(ae);
}

DEFINE_TEST(test_write_disk_sparse)
{
	struct archive *ad;


	/*
	 * The return values, etc, of the write data functions
	 * shouldn't change regardless of whether we've requested
	 * sparsification.  (The performance and pattern of actual
	 * write calls to the disk should vary, of course, but the
	 * client program shouldn't see any difference.)
	 */
	assert((ad = archive_write_disk_new()) != NULL);
        archive_write_disk_set_options(ad, 0);
	verify_write_data(ad, 0);
	verify_write_data_block(ad, 0);
	assertEqualInt(0, archive_write_free(ad));

	assert((ad = archive_write_disk_new()) != NULL);
        archive_write_disk_set_options(ad, ARCHIVE_EXTRACT_SPARSE);
	verify_write_data(ad, 1);
	verify_write_data_block(ad, 1);
	assertEqualInt(0, archive_write_free(ad));

}