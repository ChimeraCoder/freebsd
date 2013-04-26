
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
31,157,144,199,226,'T',' ',16,'+','O',187,' ',232,6,'$',20,0,160,'!',156,
'!',244,154,'0','l',216,208,5,128,128,20,'3','R',12,160,177,225,2,141,'T',
164,4,'I',194,164,136,148,16,'(',';',170,'\\',201,178,165,203,151,'0','c',
202,156,'I',179,166,205,155,'8','s',234,220,201,179,167,207,159,'@',127,2};

DEFINE_TEST(test_read_format_cpio_bin_Z)
{
	struct archive_entry *ae;
	struct archive *a;
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_open_memory(a, archive, sizeof(archive)));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualInt(1, archive_file_count(a));
	failure("archive_filter_name(a, 0)=\"%s\"",
	    archive_filter_name(a, 0));
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_COMPRESS);
	assertEqualString(archive_filter_name(a, 0), "compress (.Z)");
	failure("archive_format_name(a)=\"%s\"",
	    archive_format_name(a));
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_CPIO_BIN_LE);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}