
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
archive_read_support_format_all(struct archive *a)
{
	archive_check_magic(a, ARCHIVE_READ_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_read_support_format_all");

	/* TODO: It would be nice to compute the ordering
	 * here automatically so that people who enable just
	 * a few formats can still get the benefits.  That
	 * may just require the format registration to include
	 * a "maximum read-ahead" value (anything that uses seek
	 * would be essentially infinite read-ahead).  The core
	 * bid management can then sort the bidders before calling
	 * them.
	 *
	 * If you implement the above, please return the list below
	 * to alphabetic order.
	 */

	/*
	 * These bidders are all pretty cheap; they just examine a
	 * small initial part of the archive.  If one of these bids
	 * high, we can maybe avoid running any of the more expensive
	 * bidders below.
	 */
	archive_read_support_format_ar(a);
	archive_read_support_format_cpio(a);
	archive_read_support_format_empty(a);
	archive_read_support_format_lha(a);
	archive_read_support_format_mtree(a);
	archive_read_support_format_tar(a);
	archive_read_support_format_xar(a);

	/*
	 * Install expensive bidders last.  By doing them last, we
	 * increase the chance that a high bid from someone else will
	 * make it unnecessary for these to do anything at all.
	 */
	/* These three have potentially large look-ahead. */
	archive_read_support_format_7zip(a);
	archive_read_support_format_cab(a);
	archive_read_support_format_rar(a);
	archive_read_support_format_iso9660(a);
	/* Seek is really bad, since it forces the read-ahead
	 * logic to discard buffered data. */
	archive_read_support_format_zip(a);

	/* Note: We always return ARCHIVE_OK here, even if some of the
	 * above return ARCHIVE_WARN.  The intent here is to enable
	 * "as much as possible."  Clients who need specific
	 * compression should enable those individually so they can
	 * verify the level of support. */
	/* Clear any warning messages set by the above functions. */
	archive_clear_error(a);
	return (ARCHIVE_OK);
}