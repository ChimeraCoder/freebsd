
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

#include "archive.h"
#include "archive_private.h"

int
archive_read_support_format_by_code(struct archive *a, int format_code)
{
	archive_check_magic(a, ARCHIVE_READ_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_read_support_format_by_code");

	switch (format_code & ARCHIVE_FORMAT_BASE_MASK) {
	case ARCHIVE_FORMAT_7ZIP:
		return archive_read_support_format_7zip(a);
		break;
	case ARCHIVE_FORMAT_AR:
		return archive_read_support_format_ar(a);
		break;
	case ARCHIVE_FORMAT_CAB:
		return archive_read_support_format_cab(a);
		break;
	case ARCHIVE_FORMAT_CPIO:
		return archive_read_support_format_cpio(a);
		break;
	case ARCHIVE_FORMAT_ISO9660:
		return archive_read_support_format_iso9660(a);
		break;
	case ARCHIVE_FORMAT_LHA:
		return archive_read_support_format_lha(a);
		break;
	case ARCHIVE_FORMAT_MTREE:
		return archive_read_support_format_mtree(a);
		break;
	case ARCHIVE_FORMAT_RAR:
		return archive_read_support_format_rar(a);
		break;
	case ARCHIVE_FORMAT_TAR:
		return archive_read_support_format_tar(a);
		break;
	case ARCHIVE_FORMAT_XAR:
		return archive_read_support_format_xar(a);
		break;
	case ARCHIVE_FORMAT_ZIP:
		return archive_read_support_format_zip(a);
		break;
	}
	return (ARCHIVE_FATAL);
}