
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

DEFINE_TEST(test_write_disk_failures)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
	skipping("archive_write_disk interface");
#else
	struct archive_entry *ae;
	struct archive *a;
	int fd;

	/* Force the umask to something predictable. */
	assertUmask(022);

	/* A directory that we can't write to. */
	assertMakeDir("dir", 0555);

	/* Can we? */
	fd = open("dir/testfile", O_WRONLY | O_CREAT | O_BINARY, 0777);
	if (fd >= 0) {
	  /* Apparently, we can, so the test below won't work. */
	  close(fd);
	  skipping("Can't test writing to non-writable directory");
	  return;
	}

	/* Try to extract a regular file into the directory above. */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "dir/file");
	archive_entry_set_mode(ae, S_IFREG | 0755);
	archive_entry_set_size(ae, 8);
	assert((a = archive_write_disk_new()) != NULL);
        archive_write_disk_set_options(a, ARCHIVE_EXTRACT_TIME);
	archive_entry_set_mtime(ae, 123456789, 0);
	assertEqualIntA(a, ARCHIVE_FAILED, archive_write_header(a, ae));
	assertEqualIntA(a, 0, archive_write_finish_entry(a));
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
	archive_entry_free(ae);
#endif
}