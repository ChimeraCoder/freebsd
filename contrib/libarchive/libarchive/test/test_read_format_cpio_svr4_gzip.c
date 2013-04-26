
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
31,139,8,0,236,'c',217,'D',0,3,'3','0','7','0','7','0','4','0','0',181,'0',
183,'L',2,210,6,6,'&',134,169,')',' ',218,192,'8',213,2,133,'6','0','0','2',
'1','6','7','0','5','0','N','6','@',5,'&',16,202,208,212,0,';','0',130,'1',
244,24,12,160,246,17,5,136,'U',135,14,146,'`',140,144,' ','G','O',31,215,
' ','E','E','E',134,'Q',128,21,0,0,'%',215,202,221,0,2,0,0};

DEFINE_TEST(test_read_format_cpio_svr4_gzip)
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
	assertEqualInt(archive_filter_code(a, 0),
	    ARCHIVE_FILTER_GZIP);
	assertEqualInt(archive_format(a),
	    ARCHIVE_FORMAT_CPIO_SVR4_NOCRC);
	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}