
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
__FBSDID("$FreeBSD");

#include <locale.h>

/*
 * Test "tar:compat-2x" option that enables the string conversion of
 * libarchive 2.x, which made incorrect UTF-8 form filenames for the
 * pax format on some platform the wchar_t of which was not Unicode form.
 * The option is unneeded if people have been using UTF-8 locale during
 * making tar files(in pax format).
 *
 * NOTE: The sample tar file was made with bsdtar 2.x in LANG=KOI8-R on
 * FreeBSD.
 */

DEFINE_TEST(test_compat_pax_libarchive_2x)
{
#if (defined(_WIN32) && !defined(__CYGWIN__)) \
         || defined(__STDC_ISO_10646__) || defined(__APPLE__)
	skipping("This test only for the platform the WCS of which is "
	    "not Unicode.");
#else
	struct archive *a;
	struct archive_entry *ae;
	char c;
	wchar_t wc;
	const char *refname = "test_compat_pax_libarchive_2x.tar.Z";

	/*
 	* Read incorrect format UTF-8 filename in ru_RU.KOI8-R with
	* "tar:compat-2x" option. We should correctly
	* read two filenames.
	*/
	if (NULL == setlocale(LC_ALL, "ru_RU.KOI8-R")) {
		skipping("ru_RU.KOI8-R locale not available on this system.");
		return;
	}

	/*
	 * Test if wchar_t format is the same as FreeBSD wchar_t.
	 */
	assert(-1 != wctomb(NULL, L'\0'));
	wc = (wchar_t)0xd0;
	c = 0;
	if (wctomb(&c, wc) != 1 || (unsigned char)c != 0xd0) {
		skipping("wchar_t format is different on this platform.");
		return;
	}

	extract_reference_file(refname);

	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_set_options(a, "tar:compat-2x"));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_open_filename(a, refname, 10240));

	/* Verify regular first file. */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("\xd0\xd2\xc9\xd7\xc5\xd4",
	    archive_entry_pathname(ae));
	assertEqualInt(6, archive_entry_size(ae));

	/* Verify regular second file. */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
	assertEqualString("\xf0\xf2\xe9\xf7\xe5\xf4",
	    archive_entry_pathname(ae));
	assertEqualInt(6, archive_entry_size(ae));


	/* End of archive. */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify archive format. */
	assertEqualIntA(a, ARCHIVE_FILTER_COMPRESS, archive_filter_code(a, 0));
	assertEqualIntA(a, ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE,
	    archive_format(a));

	/* Close the archive. */
	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));

	/*
	 * Without "tar:compat-2x" option.
	 * Neither first file name nor second file name can be translated
	 * to KOI8-R.
	 */
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_open_filename(a, refname, 10240));

	/* We cannot correctly read the filename. */
	assertEqualIntA(a, ARCHIVE_WARN, archive_read_next_header(a, &ae));
	assert(strcmp("\xd0\xd2\xc9\xd7\xc5\xd4",
	    archive_entry_pathname(ae)) != 0);
	assertEqualInt(6, archive_entry_size(ae));

	/* We cannot correctly read the filename. */
	assertEqualIntA(a, ARCHIVE_WARN, archive_read_next_header(a, &ae));
	assert(strcmp("\xf0\xf2\xe9\xf7\xe5\xf4",
	    archive_entry_pathname(ae)) != 0);
	assertEqualInt(6, archive_entry_size(ae));


	/* End of archive. */
	assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify archive format. */
	assertEqualIntA(a, ARCHIVE_FILTER_COMPRESS, archive_filter_code(a, 0));
	assertEqualIntA(a, ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE,
	    archive_format(a));

	/* Close the archive. */
	assertEqualInt(ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
#endif
}