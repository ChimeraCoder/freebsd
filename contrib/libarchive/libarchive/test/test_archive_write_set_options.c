
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

#define should(__a, __code, __opts) \
assertEqualInt(__code, archive_write_set_options(__a, __opts))

static void
test(int pristine)
{
	struct archive* a = archive_write_new();
	int halfempty_options_rv = pristine ? ARCHIVE_FAILED : ARCHIVE_OK;
	int known_option_rv = pristine ? ARCHIVE_FAILED : ARCHIVE_OK;

	if (!pristine) {
		archive_write_add_filter_gzip(a);
		archive_write_set_format_iso9660(a);
	}

	/* NULL and "" denote `no option', so they're ok no matter
	 * what, if any, formats are registered */
	should(a, ARCHIVE_OK, NULL);
	should(a, ARCHIVE_OK, "");

	/* unknown modules and options */
	should(a, ARCHIVE_FAILED, "fubar:snafu");
	assertEqualString("Unknown module name: `fubar'",
	    archive_error_string(a));
	should(a, ARCHIVE_FAILED, "fubar:snafu=betcha");
	assertEqualString("Unknown module name: `fubar'",
	    archive_error_string(a));

	/* unknown modules and options */
	should(a, ARCHIVE_FAILED, "snafu");
	assertEqualString("Undefined option: `snafu'",
	    archive_error_string(a));
	should(a, ARCHIVE_FAILED, "snafu=betcha");
	assertEqualString("Undefined option: `snafu'",
	    archive_error_string(a));

	/* ARCHIVE_OK with iso9660 loaded, ARCHIVE_FAILED otherwise */
	should(a, known_option_rv, "iso9660:joliet");
	if (pristine) {
		assertEqualString("Unknown module name: `iso9660'",
		    archive_error_string(a));
	}
	should(a, known_option_rv, "iso9660:joliet");
	if (pristine) {
		assertEqualString("Unknown module name: `iso9660'",
		    archive_error_string(a));
	}
	should(a, known_option_rv, "joliet");
	if (pristine) {
		assertEqualString("Undefined option: `joliet'",
		    archive_error_string(a));
	}
	should(a, known_option_rv, "!joliet");
	if (pristine) {
		assertEqualString("Undefined option: `joliet'",
		    archive_error_string(a));
	}

	should(a, ARCHIVE_OK, ",");
	should(a, ARCHIVE_OK, ",,");

	should(a, halfempty_options_rv, ",joliet");
	if (pristine) {
		assertEqualString("Undefined option: `joliet'",
		    archive_error_string(a));
	}
	should(a, halfempty_options_rv, "joliet,");
	if (pristine) {
		assertEqualString("Undefined option: `joliet'",
		    archive_error_string(a));
	}

	should(a, ARCHIVE_FAILED, "joliet,snafu");
	if (pristine) {
		assertEqualString("Undefined option: `joliet'",
		    archive_error_string(a));
	} else {
		assertEqualString("Undefined option: `snafu'",
		    archive_error_string(a));
	}

	should(a, ARCHIVE_FAILED, "iso9660:snafu");
	if (pristine) {
		assertEqualString("Unknown module name: `iso9660'",
		    archive_error_string(a));
	} else {
		assertEqualString("Undefined option: `iso9660:snafu'",
		    archive_error_string(a));
	}

	archive_write_free(a);
}

DEFINE_TEST(test_archive_write_set_options)
{
	test(1);
	test(0);
}