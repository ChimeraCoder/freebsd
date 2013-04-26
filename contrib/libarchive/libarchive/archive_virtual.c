
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

int
archive_filter_code(struct archive *a, int n)
{
	return ((a->vtable->archive_filter_code)(a, n));
}

int
archive_filter_count(struct archive *a)
{
	return ((a->vtable->archive_filter_count)(a));
}

const char *
archive_filter_name(struct archive *a, int n)
{
	return ((a->vtable->archive_filter_name)(a, n));
}

int64_t
archive_filter_bytes(struct archive *a, int n)
{
	return ((a->vtable->archive_filter_bytes)(a, n));
}

int
archive_write_close(struct archive *a)
{
	return ((a->vtable->archive_close)(a));
}

int
archive_read_close(struct archive *a)
{
	return ((a->vtable->archive_close)(a));
}

int
archive_write_fail(struct archive *a)
{
	a->state = ARCHIVE_STATE_FATAL;
	return a->state;
}

int
archive_write_free(struct archive *a)
{
	if (a == NULL)
		return (ARCHIVE_OK);
	return ((a->vtable->archive_free)(a));
}

#if ARCHIVE_VERSION_NUMBER < 4000000
/* For backwards compatibility; will be removed with libarchive 4.0. */
int
archive_write_finish(struct archive *a)
{
	return archive_write_free(a);
}
#endif

int
archive_read_free(struct archive *a)
{
	if (a == NULL)
		return (ARCHIVE_OK);
	return ((a->vtable->archive_free)(a));
}

#if ARCHIVE_VERSION_NUMBER < 4000000
/* For backwards compatibility; will be removed with libarchive 4.0. */
int
archive_read_finish(struct archive *a)
{
	return archive_read_free(a);
}
#endif

int
archive_write_header(struct archive *a, struct archive_entry *entry)
{
	++a->file_count;
	return ((a->vtable->archive_write_header)(a, entry));
}

int
archive_write_finish_entry(struct archive *a)
{
	return ((a->vtable->archive_write_finish_entry)(a));
}

ssize_t
archive_write_data(struct archive *a, const void *buff, size_t s)
{
	return ((a->vtable->archive_write_data)(a, buff, s));
}

ssize_t
archive_write_data_block(struct archive *a, const void *buff, size_t s, int64_t o)
{
	if (a->vtable->archive_write_data_block == NULL) {
		archive_set_error(a, ARCHIVE_ERRNO_MISC,
		    "archive_write_data_block not supported");
		a->state = ARCHIVE_STATE_FATAL;
		return (ARCHIVE_FATAL);
	}
	return ((a->vtable->archive_write_data_block)(a, buff, s, o));
}

int
archive_read_next_header(struct archive *a, struct archive_entry **entry)
{
	return ((a->vtable->archive_read_next_header)(a, entry));
}

int
archive_read_next_header2(struct archive *a, struct archive_entry *entry)
{
	return ((a->vtable->archive_read_next_header2)(a, entry));
}

int
archive_read_data_block(struct archive *a,
    const void **buff, size_t *s, int64_t *o)
{
	return ((a->vtable->archive_read_data_block)(a, buff, s, o));
}