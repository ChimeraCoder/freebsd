
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
31,139,8,0,'+','e',217,'D',0,3,211,211,'g',160,'9','0',0,2,'s','S','S',16,
'm','h','n','j',128,'L',195,0,131,161,129,177,177,137,129,137,185,185,161,
'!',131,129,161,129,153,161,'9',131,130,')',237,157,198,192,'P','Z','\\',
146,'X',164,160,192,'P',146,153,139,'W',29,'!','y',152,'G','`',244,'(',24,
5,163,'`',20,12,'r',0,0,226,234,'6',162,0,6,0,0};

DEFINE_TEST(test_read_format_gtar_gz)
{
	struct archive_entry *ae;
	struct archive *a;
	int r;

	assert((a = archive_read_new()) != NULL);
	assertEqualInt(ARCHIVE_OK, archive_read_support_filter_all(a));
	r = archive_read_support_filter_gzip(a);
	if (r == ARCHIVE_WARN) {
		skipping("gzip reading not fully supported on this platform");
		assertEqualInt(ARCHIVE_OK, archive_read_free(a));
		return;
	}
	assertEqualInt(ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualInt(ARCHIVE_OK,
	    archive_read_open_memory(a, archive, sizeof(archive)));
	assertEqualInt(ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualInt(1, archive_file_count(a));
	assertEqualInt(archive_filter_code(a, 0),
	    ARCHIVE_FILTER_GZIP);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_GNUTAR);
	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}