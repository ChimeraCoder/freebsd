
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

/*
 * Verify that the various archive_read_support_* functions
 * return appropriate errors when invoked on the wrong kind of
 * archive handle.
 */

typedef struct archive *constructor(void);
typedef int enabler(struct archive *);
typedef int destructor(struct archive *);

static void
test_success(constructor new_, enabler enable_, destructor free_)
{
	struct archive *a = new_();
	int result = enable_(a);
	if (result == ARCHIVE_WARN) {
		assert(NULL != archive_error_string(a));
		assertEqualIntA(a, -1, archive_errno(a));
	} else {
		assertEqualIntA(a, ARCHIVE_OK, result);
		assert(NULL == archive_error_string(a));
		assertEqualIntA(a, 0, archive_errno(a));
	}
	free_(a);
}

static void
test_failure(constructor new_, enabler enable_, destructor free_)
{
	struct archive *a = new_();
	assertEqualIntA(a, ARCHIVE_FATAL, enable_(a));
	assert(NULL != archive_error_string(a));
	assertEqualIntA(a, -1, archive_errno(a));
	free_(a);
}

static void
test_filter_or_format(enabler enable)
{
	test_success(archive_read_new, enable, archive_read_free);
	test_failure(archive_write_new, enable, archive_write_free);
	test_failure(archive_read_disk_new, enable, archive_read_free);
	test_failure(archive_write_disk_new, enable, archive_write_free);
}

DEFINE_TEST(test_archive_read_support)
{
	test_filter_or_format(archive_read_support_format_7zip);
	test_filter_or_format(archive_read_support_format_all);
	test_filter_or_format(archive_read_support_format_ar);
	test_filter_or_format(archive_read_support_format_cab);
	test_filter_or_format(archive_read_support_format_cpio);
	test_filter_or_format(archive_read_support_format_empty);
	test_filter_or_format(archive_read_support_format_iso9660);
	test_filter_or_format(archive_read_support_format_lha);
	test_filter_or_format(archive_read_support_format_mtree);
	test_filter_or_format(archive_read_support_format_tar);
	test_filter_or_format(archive_read_support_format_xar);
	test_filter_or_format(archive_read_support_format_zip);

	test_filter_or_format(archive_read_support_filter_all);
	test_filter_or_format(archive_read_support_filter_bzip2);
	test_filter_or_format(archive_read_support_filter_compress);
	test_filter_or_format(archive_read_support_filter_gzip);
	test_filter_or_format(archive_read_support_filter_lzip);
	test_filter_or_format(archive_read_support_filter_lzma);
	test_filter_or_format(archive_read_support_filter_none);
	test_filter_or_format(archive_read_support_filter_rpm);
	test_filter_or_format(archive_read_support_filter_uu);
	test_filter_or_format(archive_read_support_filter_xz);
}