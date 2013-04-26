
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


#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "archive.h"
#include "archive_private.h"
#include "archive_read_private.h"

static const unsigned char grzip_magic[] = {
	0x47, 0x52, 0x5a, 0x69, 0x70, 0x49, 0x49, 0x00,
	0x02, 0x04, 0x3a, 0x29 };

static int	grzip_bidder_bid(struct archive_read_filter_bidder *,
		    struct archive_read_filter *);
static int	grzip_bidder_init(struct archive_read_filter *);


static int
grzip_reader_free(struct archive_read_filter_bidder *self)
{
	(void)self; /* UNUSED */
	return (ARCHIVE_OK);
}

int
archive_read_support_filter_grzip(struct archive *_a)
{
	struct archive_read *a = (struct archive_read *)_a;
	struct archive_read_filter_bidder *reader;

	archive_check_magic(_a, ARCHIVE_READ_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_read_support_filter_grzip");

	if (__archive_read_get_bidder(a, &reader) != ARCHIVE_OK)
		return (ARCHIVE_FATAL);

	reader->data = NULL;
	reader->bid = grzip_bidder_bid;
	reader->init = grzip_bidder_init;
	reader->options = NULL;
	reader->free = grzip_reader_free;
	/* This filter always uses an external program. */
	archive_set_error(_a, ARCHIVE_ERRNO_MISC,
	    "Using external grzip program for grzip decompression");
	return (ARCHIVE_WARN);
}

/*
 * Bidder just verifies the header and returns the number of verified bits.
 */
static int
grzip_bidder_bid(struct archive_read_filter_bidder *self,
    struct archive_read_filter *filter)
{
	const unsigned char *p;
	ssize_t avail;

	(void)self; /* UNUSED */

	p = __archive_read_filter_ahead(filter, sizeof(grzip_magic), &avail);
	if (p == NULL || avail == 0)
		return (0);

	if (memcmp(p, grzip_magic, sizeof(grzip_magic)))
		return (0);

	return (sizeof(grzip_magic) * 8);
}

static int
grzip_bidder_init(struct archive_read_filter *self)
{
	int r;

	r = __archive_read_program(self, "grzip -d");
	/* Note: We set the format here even if __archive_read_program()
	 * above fails.  We do, after all, know what the format is
	 * even if we weren't able to read it. */
	self->code = ARCHIVE_FILTER_GRZIP;
	self->name = "grzip";
	return (r);
}