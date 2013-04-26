
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
 * Exercise time restores in archive_write_disk(), including
 * correct handling of omitted time values.
 * On FreeBSD, we also test birthtime and high-res time restores.
 */

DEFINE_TEST(test_write_disk_times)
{
	struct archive *a;
	struct archive_entry *ae;

	/* Create an archive_write_disk object. */
	assert((a = archive_write_disk_new()) != NULL);
	assertEqualInt(ARCHIVE_OK,
	    archive_write_disk_set_options(a, ARCHIVE_EXTRACT_TIME));

	/*
	 * Easy case: mtime and atime both specified.
	 */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file1");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	archive_entry_set_atime(ae, 123456, 0);
	archive_entry_set_mtime(ae, 234567, 0);
	assertEqualInt(ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualInt(ARCHIVE_OK, archive_write_finish_entry(a));
	archive_entry_free(ae);
	/* Verify */
	assertFileAtime("file1", 123456, 0);
	assertFileMtime("file1", 234567, 0);

	/*
	 * mtime specified, but not atime
	 */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file2");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	archive_entry_set_mtime(ae, 234567, 0);
	assertEqualInt(ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualInt(ARCHIVE_OK, archive_write_finish_entry(a));
	archive_entry_free(ae);
	assertFileMtime("file2", 234567, 0);
	assertFileAtimeRecent("file2");

	/*
	 * atime specified, but not mtime
	 */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file3");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	archive_entry_set_atime(ae, 345678, 0);
	assertEqualInt(ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualInt(ARCHIVE_OK, archive_write_finish_entry(a));
	archive_entry_free(ae);
	/* Verify: Current mtime and atime as specified. */
	assertFileAtime("file3", 345678, 0);
	assertFileMtimeRecent("file3");

	/*
	 * Neither atime nor mtime specified.
	 */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file4");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	assertEqualInt(ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualInt(ARCHIVE_OK, archive_write_finish_entry(a));
	archive_entry_free(ae);
	/* Verify: Current mtime and atime. */
	assertFileAtimeRecent("file4");
	assertFileMtimeRecent("file4");

#if defined(__FreeBSD__)
	/*
	 * High-res mtime and atime on FreeBSD.
	 */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file10");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	archive_entry_set_atime(ae, 1234567, 23456);
	archive_entry_set_mtime(ae, 2345678, 4567);
	assertEqualInt(ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualInt(ARCHIVE_OK, archive_write_finish_entry(a));
	archive_entry_free(ae);
	/* Verify */
	assertFileMtime("file10", 2345678, 4567);
	assertFileAtime("file10", 1234567, 23456);

	/*
	 * Birthtime, mtime and atime on FreeBSD
	 */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file11");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	archive_entry_set_atime(ae, 1234567, 23456);
	archive_entry_set_birthtime(ae, 3456789, 12345);
	/* mtime must be later than birthtime! */
	archive_entry_set_mtime(ae, 12345678, 4567);
	assertEqualInt(ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualInt(ARCHIVE_OK, archive_write_finish_entry(a));
	archive_entry_free(ae);
	/* Verify */
	assertFileAtime("file11", 1234567, 23456);
	assertFileBirthtime("file11", 3456789, 12345);
	assertFileMtime("file11", 12345678, 4567);

	/*
	 * Birthtime only on FreeBSD.
	 */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file12");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	archive_entry_set_birthtime(ae, 3456789, 12345);
	assertEqualInt(ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualInt(ARCHIVE_OK, archive_write_finish_entry(a));
	archive_entry_free(ae);
	/* Verify */
	assertFileAtimeRecent("file12");
	assertFileBirthtime("file12", 3456789, 12345);
	assertFileMtimeRecent("file12");

	/*
	 * mtime only on FreeBSD.
	 */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "file13");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	archive_entry_set_mtime(ae, 4567890, 23456);
	assertEqualInt(ARCHIVE_OK, archive_write_header(a, ae));
	assertEqualInt(ARCHIVE_OK, archive_write_finish_entry(a));
	archive_entry_free(ae);
	/* Verify */
	assertFileAtimeRecent("file13");
	assertFileBirthtime("file13", 4567890, 23456);
	assertFileMtime("file13", 4567890, 23456);
#else
	skipping("Platform-specific time restore tests");
#endif

	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
}