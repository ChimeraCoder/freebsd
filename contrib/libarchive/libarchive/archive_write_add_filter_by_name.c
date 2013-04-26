
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
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "archive.h"
#include "archive_private.h"

/* A table that maps names to functions. */
static
struct { const char *name; int (*setter)(struct archive *); } names[] =
{
	{ "b64encode",		archive_write_add_filter_b64encode },
	{ "bzip2",		archive_write_add_filter_bzip2 },
	{ "compress",		archive_write_add_filter_compress },
	{ "grzip",		archive_write_add_filter_grzip },
	{ "gzip",		archive_write_add_filter_gzip },
	{ "lrzip",		archive_write_add_filter_lrzip },
	{ "lzip",		archive_write_add_filter_lzip },
	{ "lzma",		archive_write_add_filter_lzma },
	{ "lzop",		archive_write_add_filter_lzop },
	{ "uuencode",		archive_write_add_filter_uuencode },
	{ "xz",			archive_write_add_filter_xz },
	{ NULL,			NULL }
};

int
archive_write_add_filter_by_name(struct archive *a, const char *name)
{
	int i;

	for (i = 0; names[i].name != NULL; i++) {
		if (strcmp(name, names[i].name) == 0)
			return ((names[i].setter)(a));
	}

	archive_set_error(a, EINVAL, "No such filter '%s'", name);
	a->state = ARCHIVE_STATE_FATAL;
	return (ARCHIVE_FATAL);
}