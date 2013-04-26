
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
	{ "7zip",	archive_write_set_format_7zip },
	{ "ar",		archive_write_set_format_ar_bsd },
	{ "arbsd",	archive_write_set_format_ar_bsd },
	{ "argnu",	archive_write_set_format_ar_svr4 },
	{ "arsvr4",	archive_write_set_format_ar_svr4 },
	{ "bsdtar",	archive_write_set_format_pax_restricted },
	{ "cd9660",	archive_write_set_format_iso9660 },
	{ "cpio",	archive_write_set_format_cpio },
	{ "gnutar",	archive_write_set_format_gnutar },
	{ "iso",	archive_write_set_format_iso9660 },
	{ "iso9660",	archive_write_set_format_iso9660 },
	{ "mtree",	archive_write_set_format_mtree },
	{ "mtree-classic",	archive_write_set_format_mtree_classic },
	{ "newc",	archive_write_set_format_cpio_newc },
	{ "odc",	archive_write_set_format_cpio },
	{ "oldtar",	archive_write_set_format_v7tar },
	{ "pax",	archive_write_set_format_pax },
	{ "paxr",	archive_write_set_format_pax_restricted },
	{ "posix",	archive_write_set_format_pax },
	{ "rpax",	archive_write_set_format_pax_restricted },
	{ "shar",	archive_write_set_format_shar },
	{ "shardump",	archive_write_set_format_shar_dump },
	{ "ustar",	archive_write_set_format_ustar },
	{ "v7tar",	archive_write_set_format_v7tar },
	{ "v7",		archive_write_set_format_v7tar },
	{ "xar",	archive_write_set_format_xar },
	{ "zip",	archive_write_set_format_zip },
	{ NULL,		NULL }
};

int
archive_write_set_format_by_name(struct archive *a, const char *name)
{
	int i;

	for (i = 0; names[i].name != NULL; i++) {
		if (strcmp(name, names[i].name) == 0)
			return ((names[i].setter)(a));
	}

	archive_set_error(a, EINVAL, "No such format '%s'", name);
	a->state = ARCHIVE_STATE_FATAL;
	return (ARCHIVE_FATAL);
}