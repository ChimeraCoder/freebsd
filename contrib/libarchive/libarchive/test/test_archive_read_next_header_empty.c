
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

static void
test_empty_file1(void)
{
	struct archive* a = archive_read_new();

	/* Try opening an empty file with the raw handler. */
	assertEqualInt(ARCHIVE_OK, archive_read_support_format_raw(a));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	/* Raw handler doesn't support empty files. */
	assertEqualInt(ARCHIVE_FATAL, archive_read_open_filename(a, "emptyfile", 0));
	assert(NULL != archive_error_string(a));

	archive_read_free(a);
}

static void
test_empty_file2(void)
{
	struct archive* a = archive_read_new();
	struct archive_entry* e;

	/* Try opening an empty file with raw and empty handlers. */
	assertEqualInt(ARCHIVE_OK, archive_read_support_format_raw(a));
	assertEqualInt(ARCHIVE_OK, archive_read_support_format_empty(a));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	assertEqualInt(ARCHIVE_OK, archive_read_open_filename(a, "emptyfile", 0));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	assertEqualInt(ARCHIVE_EOF, archive_read_next_header(a, &e));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	archive_read_free(a);
}

static void
test_empty_tarfile(void)
{
	struct archive* a = archive_read_new();
	struct archive_entry* e;

	/* Try opening an empty file with raw and empty handlers. */
	assertEqualInt(ARCHIVE_OK, archive_read_support_format_tar(a));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	assertEqualInt(ARCHIVE_OK, archive_read_open_filename(a, "empty.tar", 0));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	assertEqualInt(ARCHIVE_EOF, archive_read_next_header(a, &e));
	assertEqualInt(0, archive_errno(a));
	assertEqualString(NULL, archive_error_string(a));

	archive_read_free(a);
}

/* 512 zero bytes. */
static char nulls[512];

DEFINE_TEST(test_archive_read_next_header_empty)
{
	FILE *f;

	/* Create an empty file. */
	f = fopen("emptyfile", "wb");
	fclose(f);

	/* Create a file with 512 zero bytes. */
	f = fopen("empty.tar", "wb");
	assertEqualInt(512, fwrite(nulls, 1, 512, f));
	fclose(f);

	test_empty_file1();
	test_empty_file2();
	test_empty_tarfile();

}