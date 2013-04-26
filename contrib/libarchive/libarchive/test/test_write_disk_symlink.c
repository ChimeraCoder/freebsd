
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
 * Exercise symlink recreation.
 */
DEFINE_TEST(test_write_disk_symlink)
{
	static const char data[]="abcdefghijklmnopqrstuvwxyz";
	struct archive *ad;
	struct archive_entry *ae;
	int r;

	if (!canSymlink()) {
		skipping("Symlinks not supported");
		return;
	}

	/* Write entries to disk. */
	assert((ad = archive_write_disk_new()) != NULL);

	/*
	 * First, create a regular file then a symlink to that file.
	 */

	/* Regular file: link1a */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "link1a");
	archive_entry_set_mode(ae, AE_IFREG | 0755);
	archive_entry_set_size(ae, sizeof(data));
	assertEqualIntA(ad, 0, archive_write_header(ad, ae));
	assertEqualInt(sizeof(data),
	    archive_write_data(ad, data, sizeof(data)));
	assertEqualIntA(ad, 0, archive_write_finish_entry(ad));
	archive_entry_free(ae);

	/* Symbolic Link: link1b -> link1a */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "link1b");
	archive_entry_set_mode(ae, AE_IFLNK | 0642);
	archive_entry_set_size(ae, 0);
	archive_entry_copy_symlink(ae, "link1a");
	assertEqualIntA(ad, 0, r = archive_write_header(ad, ae));
	if (r >= ARCHIVE_WARN)
		assertEqualIntA(ad, 0, archive_write_finish_entry(ad));
	archive_entry_free(ae);

	/*
	 * We should be able to do this in the other order as well,
	 * of course.
	 */

	/* Symbolic link: link2b -> link2a */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "link2b");
	archive_entry_set_mode(ae, AE_IFLNK | 0642);
	archive_entry_unset_size(ae);
	archive_entry_copy_symlink(ae, "link2a");
	assertEqualIntA(ad, 0, r = archive_write_header(ad, ae));
	if (r >= ARCHIVE_WARN) {
		assertEqualInt(ARCHIVE_WARN,
		    archive_write_data(ad, data, sizeof(data)));
		assertEqualIntA(ad, 0, archive_write_finish_entry(ad));
	}
	archive_entry_free(ae);

	/* File: link2a */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "link2a");
	archive_entry_set_mode(ae, AE_IFREG | 0755);
	archive_entry_set_size(ae, sizeof(data));
	assertEqualIntA(ad, 0, archive_write_header(ad, ae));
	assertEqualInt(sizeof(data),
	    archive_write_data(ad, data, sizeof(data)));
	assertEqualIntA(ad, 0, archive_write_finish_entry(ad));
	archive_entry_free(ae);

	assertEqualInt(ARCHIVE_OK, archive_write_free(ad));

	/* Test the entries on disk. */

	/* Test #1 */
	assertIsReg("link1a", -1);
	assertFileSize("link1a", sizeof(data));
	assertFileNLinks("link1a", 1);
	assertIsSymlink("link1b", "link1a");

	/* Test #2: Should produce identical results to test #1 */
	assertIsReg("link2a", -1);
	assertFileSize("link2a", sizeof(data));
	assertFileNLinks("link2a", 1);
	assertIsSymlink("link2b", "link2a");
}