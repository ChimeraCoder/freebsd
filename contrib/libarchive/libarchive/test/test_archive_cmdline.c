
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

#define __LIBARCHIVE_TEST
#include "archive_cmdline_private.h"

DEFINE_TEST(test_archive_cmdline)
{
	struct archive_cmdline *cl;

	/* Command name only. */
	assert((cl = __archive_cmdline_allocate()) != NULL);
	if (cl == NULL)
		return;
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_parse(cl, "gzip"));
	assertEqualInt(1, cl->argc);
	assertEqualString("gzip", cl->path);
	assertEqualString("gzip", cl->argv[0]);
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_free(cl));

	assert((cl = __archive_cmdline_allocate()) != NULL);
	if (cl == NULL)
		return;
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_parse(cl, "gzip "));
	assertEqualInt(1, cl->argc);
	failure("path should not include a space character");
	assertEqualString("gzip", cl->path);
	failure("arg0 should not include a space character");
	assertEqualString("gzip", cl->argv[0]);
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_free(cl));

	assert((cl = __archive_cmdline_allocate()) != NULL);
	if (cl == NULL)
		return;
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_parse(cl,
	    "/usr/bin/gzip "));
	assertEqualInt(1, cl->argc);
	failure("path should be a full path");
	assertEqualString("/usr/bin/gzip", cl->path);
	failure("arg0 should not be a full path");
	assertEqualString("gzip", cl->argv[0]);
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_free(cl));

	/* A command line includes space characer. */
	assert((cl = __archive_cmdline_allocate()) != NULL);
	if (cl == NULL)
		return;
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_parse(cl, "\"gzip \""));
	assertEqualInt(1, cl->argc);
	failure("path should include a space character");
	assertEqualString("gzip ", cl->path);
	failure("arg0 should include a space character");
	assertEqualString("gzip ", cl->argv[0]);
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_free(cl));

	/* A command line includes space characer: pattern 2.*/
	assert((cl = __archive_cmdline_allocate()) != NULL);
	if (cl == NULL)
		return;
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_parse(cl, "\"gzip \"x"));
	assertEqualInt(1, cl->argc);
	failure("path should include a space character");
	assertEqualString("gzip x", cl->path);
	failure("arg0 should include a space character");
	assertEqualString("gzip x", cl->argv[0]);
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_free(cl));

	/* A command line includes space characer: pattern 3.*/
	assert((cl = __archive_cmdline_allocate()) != NULL);
	if (cl == NULL)
		return;
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_parse(cl,
	    "\"gzip \"x\" s \""));
	assertEqualInt(1, cl->argc);
	failure("path should include a space character");
	assertEqualString("gzip x s ", cl->path);
	failure("arg0 should include a space character");
	assertEqualString("gzip x s ", cl->argv[0]);
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_free(cl));

	/* A command line includes space characer: pattern 4.*/
	assert((cl = __archive_cmdline_allocate()) != NULL);
	if (cl == NULL)
		return;
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_parse(cl,
	    "\"gzip\\\" \""));
	assertEqualInt(1, cl->argc);
	failure("path should include a space character");
	assertEqualString("gzip\" ", cl->path);
	failure("arg0 should include a space character");
	assertEqualString("gzip\" ", cl->argv[0]);
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_free(cl));

	/* A command name with a argument. */
	assert((cl = __archive_cmdline_allocate()) != NULL);
	if (cl == NULL)
		return;
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_parse(cl, "gzip -d"));
	assertEqualInt(2, cl->argc);
	assertEqualString("gzip", cl->path);
	assertEqualString("gzip", cl->argv[0]);
	assertEqualString("-d", cl->argv[1]);
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_free(cl));

	/* A command name with two arguments. */
	assert((cl = __archive_cmdline_allocate()) != NULL);
	if (cl == NULL)
		return;
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_parse(cl, "gzip -d -q"));
	assertEqualInt(3, cl->argc);
	assertEqualString("gzip", cl->path);
	assertEqualString("gzip", cl->argv[0]);
	assertEqualString("-d", cl->argv[1]);
	assertEqualString("-q", cl->argv[2]);
	assertEqualInt(ARCHIVE_OK, __archive_cmdline_free(cl));
}