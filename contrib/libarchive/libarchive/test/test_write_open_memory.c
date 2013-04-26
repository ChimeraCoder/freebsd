
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

/* Try to force archive_write_open_memory.c to write past the end of an array. */
static unsigned char buff[16384];

DEFINE_TEST(test_write_open_memory)
{
	unsigned int i;
	struct archive *a;
	struct archive_entry *ae;
	const char *name="/tmp/test";

	/* Create a simple archive_entry. */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_set_pathname(ae, name);
	archive_entry_set_mode(ae, S_IFREG);
	assertEqualString(archive_entry_pathname(ae), name);

	/* Try writing with different buffer sizes. */
	/* Make sure that we get failure on too-small buffers, success on
	 * large enough ones. */
	for (i = 100; i < 1600; i++) {
		size_t used;
		size_t blocksize = 94;
		assert((a = archive_write_new()) != NULL);
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_set_format_ustar(a));
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_set_bytes_in_last_block(a, 1));
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_set_bytes_per_block(a, (int)blocksize));
		buff[i] = 0xAE;
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_open_memory(a, buff, i, &used));
		/* If buffer is smaller than a tar header, this should fail. */
		if (i < (511/blocksize)*blocksize)
			assertEqualIntA(a, ARCHIVE_FATAL,
			    archive_write_header(a,ae));
		else
			assertEqualIntA(a, ARCHIVE_OK,
			    archive_write_header(a, ae));
		/* If buffer is smaller than a tar header plus 1024 byte
		 * end-of-archive marker, then this should fail. */
		failure("buffer size=%d\n", (int)i);
		if (i < 1536)
			assertEqualIntA(a, ARCHIVE_FATAL,
			    archive_write_close(a));
		else {
			assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
			assertEqualInt(used, archive_filter_bytes(a, -1));
			assertEqualInt(archive_filter_bytes(a, -1),
			    archive_filter_bytes(a, 0));
		}
		assertEqualInt(ARCHIVE_OK, archive_write_free(a));
		assertEqualInt(buff[i], 0xAE);
		assert(used <= i);
	}
	archive_entry_free(ae);
}