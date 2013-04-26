
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

#include "archive_platform.h"
__FBSDID("$FreeBSD$");

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "archive.h"
#include "archive_private.h"

/* A table that maps filter codes to functions. */
static
struct { int code; int (*setter)(struct archive *); } codes[] =
{
	{ ARCHIVE_FILTER_NONE,		archive_write_add_filter_none },
	{ ARCHIVE_FILTER_GZIP,		archive_write_add_filter_gzip },
	{ ARCHIVE_FILTER_BZIP2,		archive_write_add_filter_bzip2 },
	{ ARCHIVE_FILTER_COMPRESS,	archive_write_add_filter_compress },
	{ ARCHIVE_FILTER_GRZIP,		archive_write_add_filter_grzip },
	{ ARCHIVE_FILTER_LRZIP,		archive_write_add_filter_lrzip },
	{ ARCHIVE_FILTER_LZIP,		archive_write_add_filter_lzip },
	{ ARCHIVE_FILTER_LZMA,		archive_write_add_filter_lzma },
	{ ARCHIVE_FILTER_LZOP,		archive_write_add_filter_lzip },
	{ ARCHIVE_FILTER_UU,		archive_write_add_filter_uuencode },
	{ ARCHIVE_FILTER_XZ,		archive_write_add_filter_xz },
	{ -1,			NULL }
};

int
archive_write_add_filter(struct archive *a, int code)
{
	int i;

	for (i = 0; codes[i].code != -1; i++) {
		if (code == codes[i].code)
			return ((codes[i].setter)(a));
	}

	archive_set_error(a, EINVAL, "No such filter");
	return (ARCHIVE_FATAL);
}