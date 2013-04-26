
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

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "archive_entry.h"
#include "archive_entry_private.h"

const char *
archive_entry_strmode(struct archive_entry *entry)
{
	static const mode_t permbits[] =
	    { 0400, 0200, 0100, 0040, 0020, 0010, 0004, 0002, 0001 };
	char *bp = entry->strmode;
	mode_t mode;
	int i;

	/* Fill in a default string, then selectively override. */
	strcpy(bp, "?rwxrwxrwx ");

	mode = archive_entry_mode(entry);
	switch (archive_entry_filetype(entry)) {
	case AE_IFREG:  bp[0] = '-'; break;
	case AE_IFBLK:  bp[0] = 'b'; break;
	case AE_IFCHR:  bp[0] = 'c'; break;
	case AE_IFDIR:  bp[0] = 'd'; break;
	case AE_IFLNK:  bp[0] = 'l'; break;
	case AE_IFSOCK: bp[0] = 's'; break;
	case AE_IFIFO:  bp[0] = 'p'; break;
	default:
		if (archive_entry_hardlink(entry) != NULL) {
			bp[0] = 'h';
			break;
		}
	}

	for (i = 0; i < 9; i++)
		if (!(mode & permbits[i]))
			bp[i+1] = '-';

	if (mode & S_ISUID) {
		if (mode & 0100) bp[3] = 's';
		else bp[3] = 'S';
	}
	if (mode & S_ISGID) {
		if (mode & 0010) bp[6] = 's';
		else bp[6] = 'S';
	}
	if (mode & S_ISVTX) {
		if (mode & 0001) bp[9] = 't';
		else bp[9] = 'T';
	}
	if (archive_entry_acl_count(entry, ARCHIVE_ENTRY_ACL_TYPE_ACCESS))
		bp[10] = '+';

	return (bp);
}