
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

#include "archive_read_private.h"
#include "archive_options_private.h"

static int	archive_set_format_option(struct archive *a,
		    const char *m, const char *o, const char *v);
static int	archive_set_filter_option(struct archive *a,
		    const char *m, const char *o, const char *v);
static int	archive_set_option(struct archive *a,
		    const char *m, const char *o, const char *v);

int
archive_read_set_format_option(struct archive *a, const char *m, const char *o,
    const char *v)
{
	return _archive_set_option(a, m, o, v,
	    ARCHIVE_READ_MAGIC, "archive_read_set_format_option",
	    archive_set_format_option);
}

int
archive_read_set_filter_option(struct archive *a, const char *m, const char *o,
    const char *v)
{
	return _archive_set_option(a, m, o, v,
	    ARCHIVE_READ_MAGIC, "archive_read_set_filter_option",
	    archive_set_filter_option);
}

int
archive_read_set_option(struct archive *a, const char *m, const char *o,
    const char *v)
{
	return _archive_set_option(a, m, o, v,
	    ARCHIVE_READ_MAGIC, "archive_read_set_option",
	    archive_set_option);
}

int
archive_read_set_options(struct archive *a, const char *options)
{
	return _archive_set_options(a, options,
	    ARCHIVE_READ_MAGIC, "archive_read_set_options",
	    archive_set_option);
}

static int
archive_set_format_option(struct archive *_a, const char *m, const char *o,
    const char *v)
{
	struct archive_read *a = (struct archive_read *)_a;
	struct archive_format_descriptor *format;
	size_t i;
	int r, rv = ARCHIVE_WARN;

	for (i = 0; i < sizeof(a->formats)/sizeof(a->formats[0]); i++) {
		format = &a->formats[i];
		if (format == NULL || format->options == NULL ||
		    format->name == NULL)
			/* This format does not support option. */
			continue;
		if (m != NULL && strcmp(format->name, m) != 0)
			continue;

		a->format = format;
		r = format->options(a, o, v);
		a->format = NULL;

		if (r == ARCHIVE_FATAL)
			return (ARCHIVE_FATAL);

		if (m != NULL)
			return (r);

		if (r == ARCHIVE_OK)
			rv = ARCHIVE_OK;
	}
	/* If the format name didn't match, return a special code for
	 * _archive_set_option[s]. */
	if (rv == ARCHIVE_WARN && m != NULL)
		rv = ARCHIVE_WARN - 1;
	return (rv);
}

static int
archive_set_filter_option(struct archive *_a, const char *m, const char *o,
    const char *v)
{
	struct archive_read *a = (struct archive_read *)_a;
	struct archive_read_filter *filter;
	struct archive_read_filter_bidder *bidder;
	int r, rv = ARCHIVE_WARN;

	for (filter = a->filter; filter != NULL; filter = filter->upstream) {
		bidder = filter->bidder;
		if (bidder == NULL)
			continue;
		if (bidder->options == NULL)
			/* This bidder does not support option */
			continue;
		if (m != NULL && strcmp(filter->name, m) != 0)
			continue;

		r = bidder->options(bidder, o, v);

		if (r == ARCHIVE_FATAL)
			return (ARCHIVE_FATAL);

		if (m != NULL)
			return (r);

		if (r == ARCHIVE_OK)
			rv = ARCHIVE_OK;
	}
	/* If the filter name didn't match, return a special code for
	 * _archive_set_option[s]. */
	if (rv == ARCHIVE_WARN && m != NULL)
		rv = ARCHIVE_WARN - 1;
	return (rv);
}

static int
archive_set_option(struct archive *a, const char *m, const char *o,
    const char *v)
{
	return _archive_set_either_option(a, m, o, v,
	    archive_set_format_option,
	    archive_set_filter_option);
}