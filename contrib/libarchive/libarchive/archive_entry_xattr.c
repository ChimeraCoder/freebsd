
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
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_LINUX_FS_H
#include <linux/fs.h>	/* for Linux file flags */
#endif
/*
 * Some Linux distributions have both linux/ext2_fs.h and ext2fs/ext2_fs.h.
 * As the include guards don't agree, the order of include is important.
 */
#ifdef HAVE_LINUX_EXT2_FS_H
#include <linux/ext2_fs.h>	/* for Linux file flags */
#endif
#if defined(HAVE_EXT2FS_EXT2_FS_H) && !defined(__CYGWIN__)
#include <ext2fs/ext2_fs.h>	/* for Linux file flags */
#endif
#include <stddef.h>
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif

#include "archive.h"
#include "archive_entry.h"
#include "archive_private.h"
#include "archive_entry_private.h"

/*
 * extended attribute handling
 */

void
archive_entry_xattr_clear(struct archive_entry *entry)
{
	struct ae_xattr	*xp;

	while (entry->xattr_head != NULL) {
		xp = entry->xattr_head->next;
		free(entry->xattr_head->name);
		free(entry->xattr_head->value);
		free(entry->xattr_head);
		entry->xattr_head = xp;
	}

	entry->xattr_head = NULL;
}

void
archive_entry_xattr_add_entry(struct archive_entry *entry,
	const char *name, const void *value, size_t size)
{
	struct ae_xattr	*xp;

	for (xp = entry->xattr_head; xp != NULL; xp = xp->next)
		;

	if ((xp = (struct ae_xattr *)malloc(sizeof(struct ae_xattr))) == NULL)
		/* XXX Error XXX */
		return;

	xp->name = strdup(name);
	if ((xp->value = malloc(size)) != NULL) {
		memcpy(xp->value, value, size);
		xp->size = size;
	} else
		xp->size = 0;

	xp->next = entry->xattr_head;
	entry->xattr_head = xp;
}


/*
 * returns number of the extended attribute entries
 */
int
archive_entry_xattr_count(struct archive_entry *entry)
{
	struct ae_xattr *xp;
	int count = 0;

	for (xp = entry->xattr_head; xp != NULL; xp = xp->next)
		count++;

	return count;
}

int
archive_entry_xattr_reset(struct archive_entry * entry)
{
	entry->xattr_p = entry->xattr_head;

	return archive_entry_xattr_count(entry);
}

int
archive_entry_xattr_next(struct archive_entry * entry,
	const char **name, const void **value, size_t *size)
{
	if (entry->xattr_p) {
		*name = entry->xattr_p->name;
		*value = entry->xattr_p->value;
		*size = entry->xattr_p->size;

		entry->xattr_p = entry->xattr_p->next;

		return (ARCHIVE_OK);
	} else {
		*name = NULL;
		*value = NULL;
		*size = (size_t)0;
		return (ARCHIVE_WARN);
	}
}

/*
 * end of xattr handling
 */