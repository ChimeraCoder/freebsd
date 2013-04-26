
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
31,157,144,'.',0,8,28,'H',176,160,193,131,8,19,'*','\\',200,176,'!','B',24,
16,'o',212,168,1,2,0,196,24,18,'a','T',188,152,'q','#',196,143,' ','5',198,
128,'1','c',6,13,24,'4','0',206,176,1,2,198,200,26,'6','b',0,0,'Q',195,161,
205,155,'8','s',234,4,'P','g',14,157,'0','r',',',194,160,147,166,205,206,
132,'D',141,30,'=',24,'R',163,'P',144,21,151,'J',157,'J',181,170,213,171,
'X',179,'j',221,202,181,171,215,175,'`',195,138,29,'K',182,172,217,179,'h',
211,170,']',203,182,173,219,183,'g',1};

DEFINE_TEST(test_read_format_tz)
{
	struct archive_entry *ae;
	struct archive *a;
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_open_memory(a, archive, sizeof(archive)));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualInt(1, archive_file_count(a));
	failure("archive_filter_name(a, 0)=\"%s\"",
	    archive_filter_name(a, 0));
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_COMPRESS);
	failure("archive_format_name(a)=\"%s\"", archive_format_name(a));
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_USTAR);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}