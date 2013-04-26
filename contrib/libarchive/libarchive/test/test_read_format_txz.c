
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
253, 55,122, 88, 90,  0,  0,  4,230,214,180, 70,  2,  0, 33,  1,
 22,  0,  0,  0,116, 47,229,163,224,  5,255,  0, 73, 93,  0, 23,
  0, 51, 80, 24,164,204,238, 45, 77, 28,191, 13,144,  8, 10, 70,
  5,173,215, 47,132,237,145,162, 96,  6,131,168,152,  8,135,161,
189, 73,110,132, 27,195, 52,109,203, 22, 17,168,211, 18,181, 76,
 93,120, 88,154,155,244,141,193,206,170,224, 80,137,134, 67,  1,
  9,123,121,188,247, 28,139,  0,  0,  0,  0,  0,112,184, 17,  5,
103, 16,  8, 73,  0,  1,101,128, 12,  0,  0,  0, 30, 69, 92, 96,
177,196,103,251,  2,  0,  0,  0,  0,  4, 89, 90
};

DEFINE_TEST(test_read_format_txz)
{
	struct archive_entry *ae;
	struct archive *a;
	int r;

	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	r = archive_read_support_filter_xz(a);
	if (r == ARCHIVE_WARN) {
		skipping("xz reading not fully supported on this platform");
		assertEqualInt(ARCHIVE_OK, archive_read_free(a));
		return;
	}
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_open_memory(a, archive, sizeof(archive)));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualInt(1, archive_file_count(a));
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_XZ);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_USTAR);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}