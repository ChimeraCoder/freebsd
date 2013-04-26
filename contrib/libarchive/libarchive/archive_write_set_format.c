
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

/* A table that maps format codes to functions. */
static
struct { int code; int (*setter)(struct archive *); } codes[] =
{
	{ ARCHIVE_FORMAT_7ZIP,		archive_write_set_format_7zip },
	{ ARCHIVE_FORMAT_CPIO,		archive_write_set_format_cpio },
	{ ARCHIVE_FORMAT_CPIO_POSIX,	archive_write_set_format_cpio },
	{ ARCHIVE_FORMAT_CPIO_SVR4_NOCRC,	archive_write_set_format_cpio_newc },
	{ ARCHIVE_FORMAT_ISO9660,	archive_write_set_format_iso9660 },
	{ ARCHIVE_FORMAT_MTREE,		archive_write_set_format_mtree },
	{ ARCHIVE_FORMAT_SHAR,		archive_write_set_format_shar },
	{ ARCHIVE_FORMAT_SHAR_BASE,	archive_write_set_format_shar },
	{ ARCHIVE_FORMAT_SHAR_DUMP,	archive_write_set_format_shar_dump },
	{ ARCHIVE_FORMAT_TAR,	archive_write_set_format_pax_restricted },
	{ ARCHIVE_FORMAT_TAR_GNUTAR,	archive_write_set_format_gnutar },
	{ ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE, archive_write_set_format_pax },
	{ ARCHIVE_FORMAT_TAR_PAX_RESTRICTED,
				archive_write_set_format_pax_restricted },
	{ ARCHIVE_FORMAT_TAR_USTAR,	archive_write_set_format_ustar },
	{ ARCHIVE_FORMAT_XAR,		archive_write_set_format_xar },
	{ ARCHIVE_FORMAT_ZIP,	archive_write_set_format_zip },
	{ 0,		NULL }
};

int
archive_write_set_format(struct archive *a, int code)
{
	int i;

	for (i = 0; codes[i].code != 0; i++) {
		if (code == codes[i].code)
			return ((codes[i].setter)(a));
	}

	archive_set_error(a, EINVAL, "No such format");
	return (ARCHIVE_FATAL);
}