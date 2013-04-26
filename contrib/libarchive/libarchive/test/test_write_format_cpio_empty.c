
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
 * Check that an "empty" cpio archive is correctly created.
 */

/* Here's what an empty cpio archive should look like. */
static char ref[] =
"070707"  /* Magic number */
"000000"  /* Dev = 0 */
"000000"  /* ino = 0 */
"000000"  /* mode = 0 */
"000000"  /* uid = 0 */
"000000"  /* gid = 0 */
"000001"  /* nlink = 1 */
"000000"  /* rdev = 0 */
"00000000000" /* mtime = 0 */
"000013"  /* Namesize = 11 */
"00000000000" /* filesize = 0 */
"TRAILER!!!\0"; /* Name */

DEFINE_TEST(test_write_format_cpio_empty)
{
	struct archive *a;
	char buff[2048];
	size_t used;

	/* Create a new archive in memory. */
	assert((a = archive_write_new()) != NULL);
	assertA(0 == archive_write_set_format_cpio(a));
	assertA(0 == archive_write_add_filter_none(a));
	/* 1-byte block size ensures we see only the required bytes. */
	/* We're not testing the padding here. */
	assertA(0 == archive_write_set_bytes_per_block(a, 1));
	assertA(0 == archive_write_set_bytes_in_last_block(a, 1));
	assertA(0 == archive_write_open_memory(a, buff, sizeof(buff), &used));

	/* Close out the archive. */
	assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));

	failure("Empty cpio archive should be exactly 87 bytes, was %d.", used);
	assert(used == 87);
	failure("Empty cpio archive is incorrectly formatted.");
	assertEqualMem(buff, ref, 87);
}