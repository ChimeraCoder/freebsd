
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

static unsigned char archive[] = {
 93,  0,  0,128,  0,255,255,255,255,255,255,255,255,  0, 99,156,
 62,160, 67,124,230, 93,220,235,118, 29, 75, 27,226,158, 67,149,
151, 96, 22, 54,198,209, 63,104,209,148,249,238, 71,187,201,243,
162,  1, 42, 47, 43,178, 35, 90,  6,156,208, 74,107, 91,229,126,
  5, 85,255,136,255, 64,  0
};

DEFINE_TEST(test_read_format_cpio_bin_lzma)
{
	struct archive_entry *ae;
	struct archive *a;
	int r;

	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	r = archive_read_support_filter_lzma(a);
	if (r == ARCHIVE_WARN) {
		skipping("lzma reading not fully supported on this platform");
		assertEqualInt(ARCHIVE_OK, archive_read_free(a));
		return;
	}
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_open_memory(a, archive, sizeof(archive)));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_LZMA);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_CPIO_BIN_LE);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}