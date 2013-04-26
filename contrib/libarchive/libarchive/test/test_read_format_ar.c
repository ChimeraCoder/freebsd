
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


DEFINE_TEST(test_read_format_ar)
{
	char buff[64];
	const char reffile[] = "test_read_format_ar.ar";
	struct archive_entry *ae;
	struct archive *a;

	extract_reference_file(reffile);
	assert((a = archive_read_new()) != NULL);
	assertA(0 == archive_read_support_filter_all(a));
	assertA(0 == archive_read_support_format_all(a));
	assertA(0 == archive_read_open_filename(a, reffile, 7));

	/* Filename table.  */
	assertA(0 == archive_read_next_header(a, &ae));
	assertEqualString("//", archive_entry_pathname(ae));
	assertEqualInt(0, archive_entry_mtime(ae));
	assertEqualInt(0, archive_entry_uid(ae));
	assertEqualInt(0, archive_entry_gid(ae));
	assertEqualInt(0, archive_entry_size(ae));

	/* First Entry */
	assertA(0 == archive_read_next_header(a, &ae));
	assertEqualString("yyytttsssaaafff.o", archive_entry_pathname(ae));
	assertEqualInt(1175465652, archive_entry_mtime(ae));
	assertEqualInt(1001, archive_entry_uid(ae));
	assertEqualInt(0, archive_entry_gid(ae));
	assert(8 == archive_entry_size(ae));
	assertA(8 == archive_read_data(a, buff, 10));
	assertEqualMem(buff, "55667788", 8);

	/* Second Entry */
	assertA(0 == archive_read_next_header(a, &ae));
	assertEqualString("gghh.o", archive_entry_pathname(ae));
	assertEqualInt(1175465668, archive_entry_mtime(ae));
	assertEqualInt(1001, archive_entry_uid(ae));
	assertEqualInt(0, archive_entry_gid(ae));
	assert(4 == archive_entry_size(ae));
	assertA(4 == archive_read_data(a, buff, 10));
	assertEqualMem(buff, "3333", 4);

	/* Third Entry */
	assertA(0 == archive_read_next_header(a, &ae));
	assertEqualString("hhhhjjjjkkkkllll.o", archive_entry_pathname(ae));
	assertEqualInt(1175465713, archive_entry_mtime(ae));
	assertEqualInt(1001, archive_entry_uid(ae));
	assertEqualInt(0, archive_entry_gid(ae));
	assert(9 == archive_entry_size(ae));
	assertA(9 == archive_read_data(a, buff, 9));
	assertEqualMem(buff, "987654321", 9);

	/* Test EOF */
	assertA(1 == archive_read_next_header(a, &ae));
	assertEqualInt(4, archive_file_count(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}