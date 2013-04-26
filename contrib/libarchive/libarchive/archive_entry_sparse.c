
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
#include "archive_entry.h"
#include "archive_private.h"
#include "archive_entry_private.h"

/*
 * sparse handling
 */

void
archive_entry_sparse_clear(struct archive_entry *entry)
{
	struct ae_sparse *sp;

	while (entry->sparse_head != NULL) {
		sp = entry->sparse_head->next;
		free(entry->sparse_head);
		entry->sparse_head = sp;
	}
	entry->sparse_tail = NULL;
}

void
archive_entry_sparse_add_entry(struct archive_entry *entry,
	int64_t offset, int64_t length)
{
	struct ae_sparse *sp;

	if (offset < 0 || length < 0)
		/* Invalid value */
		return;
	if (offset + length < 0 ||
	    offset + length > archive_entry_size(entry))
		/* A value of "length" parameter is too large. */
		return;
	if ((sp = entry->sparse_tail) != NULL) {
		if (sp->offset + sp->length > offset)
			/* Invalid value. */
			return;
		if (sp->offset + sp->length == offset) {
			if (sp->offset + sp->length + length < 0)
				/* A value of "length" parameter is
				 * too large. */
				return;
			/* Expand existing sparse block size. */
			sp->length += length;
			return;
		}
	}

	if ((sp = (struct ae_sparse *)malloc(sizeof(*sp))) == NULL)
		/* XXX Error XXX */
		return;

	sp->offset = offset;
	sp->length = length;
	sp->next = NULL;

	if (entry->sparse_head == NULL)
		entry->sparse_head = entry->sparse_tail = sp;
	else {
		/* Add a new sparse block to the tail of list. */
		if (entry->sparse_tail != NULL)
			entry->sparse_tail->next = sp;
		entry->sparse_tail = sp;
	}
}


/*
 * returns number of the sparse entries
 */
int
archive_entry_sparse_count(struct archive_entry *entry)
{
	struct ae_sparse *sp;
	int count = 0;

	for (sp = entry->sparse_head; sp != NULL; sp = sp->next)
		count++;

	/*
	 * Sanity check if this entry is exactly sparse.
	 * If amount of sparse blocks is just one and it indicates the whole
	 * file data, we should remove it and return zero.
	 */
	if (count == 1) {
		sp = entry->sparse_head;
		if (sp->offset == 0 &&
		    sp->length >= archive_entry_size(entry)) {
			count = 0;
			archive_entry_sparse_clear(entry);
		}
	}

	return (count);
}

int
archive_entry_sparse_reset(struct archive_entry * entry)
{
	entry->sparse_p = entry->sparse_head;

	return archive_entry_sparse_count(entry);
}

int
archive_entry_sparse_next(struct archive_entry * entry,
	int64_t *offset, int64_t *length)
{
	if (entry->sparse_p) {
		*offset = entry->sparse_p->offset;
		*length = entry->sparse_p->length;

		entry->sparse_p = entry->sparse_p->next;

		return (ARCHIVE_OK);
	} else {
		*offset = 0;
		*length = 0;
		return (ARCHIVE_WARN);
	}
}

/*
 * end of sparse handling
 */