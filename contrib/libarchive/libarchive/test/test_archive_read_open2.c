
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

static int
open_cb(struct archive *a, void *client)
{
	(void)a; /* UNUSED */
	(void)client; /* UNUSED */
	return 0;
}
static ssize_t
read_cb(struct archive *a, void *client, const void **buff)
{
	(void)a; /* UNUSED */
	(void)client; /* UNUSED */
	(void)buff; /* UNUSED */
	return (ssize_t)0;
}
static int64_t
skip_cb(struct archive *a, void *client, int64_t request)
{
	(void)a; /* UNUSED */
	(void)client; /* UNUSED */
	(void)request; /* UNUSED */
	return (int64_t)0;
}
static int
close_cb(struct archive *a, void *client)
{
	(void)a; /* UNUSED */
	(void)client; /* UNUSED */
	return 0;
}

static void
test(int formatted, archive_open_callback *o, archive_read_callback *r,
    archive_skip_callback *s, archive_close_callback *c,
    int rv, const char *msg)
{
	struct archive* a = archive_read_new();
	if (formatted)
	    assertEqualInt(ARCHIVE_OK,
		archive_read_support_format_empty(a));
	assertEqualInt(rv,
	    archive_read_open2(a, NULL, o, r, s, c));
	assertEqualString(msg, archive_error_string(a));
	archive_read_free(a);
}

DEFINE_TEST(test_archive_read_open2)
{
	const char *no_reader =
	    "No reader function provided to archive_read_open";
	const char *no_formats = "No formats registered";

	test(1, NULL, NULL, NULL, NULL,
	    ARCHIVE_FATAL, no_reader);
	test(1, open_cb, NULL, NULL, NULL,
	    ARCHIVE_FATAL, no_reader);
	test(1, open_cb, read_cb, NULL, NULL,
	    ARCHIVE_OK, NULL);
	test(1, open_cb, read_cb, skip_cb, NULL,
	    ARCHIVE_OK, NULL);
	test(1, open_cb, read_cb, skip_cb, close_cb,
	    ARCHIVE_OK, NULL);
	test(1, NULL, read_cb, skip_cb, close_cb,
	    ARCHIVE_OK, NULL);
	test(1, open_cb, read_cb, skip_cb, NULL,
	    ARCHIVE_OK, NULL);
	test(1, NULL, read_cb, skip_cb, NULL,
	    ARCHIVE_OK, NULL);
	test(1, NULL, read_cb, NULL, NULL,
	    ARCHIVE_OK, NULL);

	test(0, NULL, NULL, NULL, NULL,
	    ARCHIVE_FATAL, no_reader);
	test(0, open_cb, NULL, NULL, NULL,
	    ARCHIVE_FATAL, no_reader);
	test(0, open_cb, read_cb, NULL, NULL,
	    ARCHIVE_FATAL, no_formats);
	test(0, open_cb, read_cb, skip_cb, NULL,
	    ARCHIVE_FATAL, no_formats);
	test(0, open_cb, read_cb, skip_cb, close_cb,
	    ARCHIVE_FATAL, no_formats);
}