
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

/* The sample has some files in a directory with a very long name. */
#define TESTPATH "abcdefghijklmnopqrstuvwxyz/"	\
	"abcdefghijklmnopqrstuvwxyz/"	\
	"abcdefghijklmnopqrstuvwxyz/"	\
	"abcdefghijklmnopqrstuvwxyz/"	\
	"abcdefghijklmnopqrstuvwxyz/"	\
	"abcdefghijklmnopqrstuvwxyz/"	\
	"abcdefghijklmnopqrstuvwxyz/"

static void test_compat_mac_1(void);
static void test_compat_mac_2(void);

/*
 * Apple shipped an extended version of GNU tar with Mac OS X 10.5
 * and earlier.
 */
static void
test_compat_mac_1(void)
{
	char name[] = "test_compat_mac-1.tar.Z";
	struct archive_entry *ae;
	struct archive *a;
	const void *attr;
	size_t attrSize;

	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	extract_reference_file(name);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_filename(a, name, 10240));

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString(TESTPATH, archive_entry_pathname(ae));
	assertEqualInt(1275688109, archive_entry_mtime(ae));
	assertEqualInt(95594, archive_entry_uid(ae));
	assertEqualString("kientzle", archive_entry_uname(ae));
	assertEqualInt(5000, archive_entry_gid(ae));
	assertEqualString("", archive_entry_gname(ae));
	assertEqualInt(040755, archive_entry_mode(ae));

	attr = archive_entry_mac_metadata(ae, &attrSize);
	assert(attr == NULL);
	assertEqualInt(0, attrSize);

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString(TESTPATH "dir/", archive_entry_pathname(ae));
	assertEqualInt(1275687611, archive_entry_mtime(ae));
	assertEqualInt(95594, archive_entry_uid(ae));
	assertEqualString("kientzle", archive_entry_uname(ae));
	assertEqualInt(5000, archive_entry_gid(ae));
	assertEqualString("", archive_entry_gname(ae));
	assertEqualInt(040755, archive_entry_mode(ae));

	attr = archive_entry_mac_metadata(ae, &attrSize);
	assert(attr != NULL);
	assertEqualInt(225, attrSize);

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString(TESTPATH "file", archive_entry_pathname(ae));
	assertEqualInt(1275687588, archive_entry_mtime(ae));
	assertEqualInt(95594, archive_entry_uid(ae));
	assertEqualString("kientzle", archive_entry_uname(ae));
	assertEqualInt(5000, archive_entry_gid(ae));
	assertEqualString("", archive_entry_gname(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	attr = archive_entry_mac_metadata(ae, &attrSize);
	assert(attr != NULL);
	assertEqualInt(225, attrSize);

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("dir/", archive_entry_pathname(ae));
	assertEqualInt(1275688064, archive_entry_mtime(ae));
	assertEqualInt(95594, archive_entry_uid(ae));
	assertEqualString("kientzle", archive_entry_uname(ae));
	assertEqualInt(5000, archive_entry_gid(ae));
	assertEqualString("", archive_entry_gname(ae));
	assertEqualInt(040755, archive_entry_mode(ae));

	attr = archive_entry_mac_metadata(ae, &attrSize);
	assert(attr != NULL);
	assertEqualInt(225, attrSize);
	assertEqualMem("\x00\x05\x16\x07\x00\x02\x00\x00Mac OS X", attr, 16);

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("file", archive_entry_pathname(ae));
	assertEqualInt(1275625860, archive_entry_mtime(ae));
	assertEqualInt(95594, archive_entry_uid(ae));
	assertEqualString("kientzle", archive_entry_uname(ae));
	assertEqualInt(5000, archive_entry_gid(ae));
	assertEqualString("", archive_entry_gname(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	attr = archive_entry_mac_metadata(ae, &attrSize);
	assert(attr != NULL);
	assertEqualInt(225, attrSize);
	assertEqualMem("\x00\x05\x16\x07\x00\x02\x00\x00Mac OS X", attr, 16);

	/* Verify the end-of-archive. */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify that the format detection worked. */
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_COMPRESS);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_GNUTAR);

	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

/*
 * Apple shipped a customized version of bsdtar starting with MacOS 10.6.
 */
static void
test_compat_mac_2(void)
{
	char name[] = "test_compat_mac-2.tar.Z";
	struct archive_entry *ae;
	struct archive *a;
	const void *attr;
	size_t attrSize;

	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	extract_reference_file(name);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_filename(a, name, 10240));

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("./", archive_entry_pathname(ae));
	assertEqualInt(1303628303, archive_entry_mtime(ae));
	assertEqualInt(501, archive_entry_uid(ae));
	assertEqualString("tim", archive_entry_uname(ae));
	assertEqualInt(20, archive_entry_gid(ae));
	assertEqualString("staff", archive_entry_gname(ae));
	assertEqualInt(040755, archive_entry_mode(ae));

	attr = archive_entry_mac_metadata(ae, &attrSize);
	assert(attr == NULL);
	assertEqualInt(0, attrSize);

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("./mydir/", archive_entry_pathname(ae));
	assertEqualInt(1303628303, archive_entry_mtime(ae));
	assertEqualInt(501, archive_entry_uid(ae));
	assertEqualString("tim", archive_entry_uname(ae));
	assertEqualInt(20, archive_entry_gid(ae));
	assertEqualString("staff", archive_entry_gname(ae));
	assertEqualInt(040755, archive_entry_mode(ae));

	attr = archive_entry_mac_metadata(ae, &attrSize);
	assert(attr != NULL);
	assertEqualInt(267, attrSize);
	assertEqualMem("\x00\x05\x16\x07\x00\x02\x00\x00Mac OS X", attr, 16);

	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("./myfile", archive_entry_pathname(ae));
	assertEqualInt(1303628303, archive_entry_mtime(ae));
	assertEqualInt(501, archive_entry_uid(ae));
	assertEqualString("tim", archive_entry_uname(ae));
	assertEqualInt(20, archive_entry_gid(ae));
	assertEqualString("staff", archive_entry_gname(ae));
	assertEqualInt(0100644, archive_entry_mode(ae));

	attr = archive_entry_mac_metadata(ae, &attrSize);
	assert(attr != NULL);
	assertEqualInt(267, attrSize);
	assertEqualMem("\x00\x05\x16\x07\x00\x02\x00\x00Mac OS X", attr, 16);

	/* Verify the end-of-archive. */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify that the format detection worked. */
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_COMPRESS);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_USTAR);

	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

DEFINE_TEST(test_compat_mac)
{
	test_compat_mac_1();
	test_compat_mac_2();
}