
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
0x5d, 0x0, 0x0, 0x80, 0x0, 0x0, 0x28, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x17, 0xb, 0xbc, 0x1c, 0x7d, 0x1, 0x95, 0xc0, 0x1d, 0x4a, 0x46, 0x9c,
0x1c, 0xc5, 0x8, 0x82, 0x10, 0xed, 0x84, 0xf6, 0xea, 0x7a, 0xfe, 0x63,
0x5a, 0x34, 0x5e, 0xf7, 0xc, 0x60, 0xd6, 0x8b, 0xc1, 0x47, 0xaf, 0x11,
0x6f, 0x18, 0x94, 0x81, 0x74, 0x8a, 0xf8, 0x47, 0xcc, 0xdd, 0xc0, 0xd9,
0x40, 0xa, 0xc3, 0xac, 0x43, 0x47, 0xb5, 0xac, 0x2b, 0x31, 0xd3, 0x6,
0xa4, 0x2c, 0x44, 0x80, 0x24, 0x4b, 0xfe, 0x43, 0x22, 0x4e, 0x14, 0x30,
0x7a, 0xef, 0x99, 0x6e, 0xf, 0x8b, 0xc1, 0x79, 0x93, 0x88, 0x54, 0x73,
0x59, 0x3f, 0xc, 0xfb, 0xee, 0x9c, 0x83, 0x49, 0x93, 0x33, 0xad, 0x44,
0xbe, 0x0};

DEFINE_TEST(test_read_format_gtar_lzma)
{
	int r;

	struct archive_entry *ae;
	struct archive *a;
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_support_filter_all(a));
	r = archive_read_support_filter_lzma(a);
	if (r == ARCHIVE_WARN) {
		skipping("lzma reading not fully supported on this platform");
		assertEqualInt(ARCHIVE_OK, archive_read_free(a));
		return;
	}

	assertEqualIntA(a, ARCHIVE_OK, r);
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_support_format_all(a));
	r = archive_read_open_memory2(a, archive, sizeof(archive), 3);
	if (r != ARCHIVE_OK) {
		skipping("Skipping LZMA compression check: %s",
		    archive_error_string(a));
		goto finish;
	}
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_next_header(a, &ae));
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_LZMA);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_GNUTAR);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
finish:
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}