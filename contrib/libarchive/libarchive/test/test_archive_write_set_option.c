
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

#define should(__a, __code, __m, __o, __v) \
assertEqualInt(__code, archive_write_set_option(__a, __m, __o, __v))

static void
test(int pristine)
{
	struct archive* a = archive_write_new();
	int known_option_rv = pristine ? ARCHIVE_FAILED : ARCHIVE_OK;

	if (!pristine) {
		archive_write_add_filter_gzip(a);
		archive_write_set_format_iso9660(a);
        }

	/* NULL and "" denote `no option', so they're ok no matter
	 * what, if any, formats are registered */
	should(a, ARCHIVE_OK, NULL, NULL, NULL);
	should(a, ARCHIVE_OK, "", "", "");

	/* unknown modules and options */
	should(a, ARCHIVE_FAILED, "fubar", "snafu", NULL);
	should(a, ARCHIVE_FAILED, "fubar", "snafu", "betcha");

	/* unknown modules and options */
	should(a, ARCHIVE_FAILED, NULL, "snafu", NULL);
	should(a, ARCHIVE_FAILED, NULL, "snafu", "betcha");

	/* ARCHIVE_OK with iso9660 loaded, ARCHIVE_WARN otherwise */
	should(a, known_option_rv, "iso9660", "joliet", NULL);
	should(a, known_option_rv, "iso9660", "joliet", NULL);
	should(a, known_option_rv, NULL, "joliet", NULL);
	should(a, known_option_rv, NULL, "joliet", NULL);

	archive_write_free(a);
}

DEFINE_TEST(test_archive_write_set_option)
{
	test(1);
	test(0);
}