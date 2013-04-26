
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "archive.h"

struct write_memory_data {
	size_t	used;
	size_t  size;
	size_t * client_size;
	unsigned char * buff;
};

static int	memory_write_close(struct archive *, void *);
static int	memory_write_open(struct archive *, void *);
static ssize_t	memory_write(struct archive *, void *, const void *buff, size_t);

/*
 * Client provides a pointer to a block of memory to receive
 * the data.  The 'size' param both tells us the size of the
 * client buffer and lets us tell the client the final size.
 */
int
archive_write_open_memory(struct archive *a, void *buff, size_t buffSize, size_t *used)
{
	struct write_memory_data *mine;

	mine = (struct write_memory_data *)malloc(sizeof(*mine));
	if (mine == NULL) {
		archive_set_error(a, ENOMEM, "No memory");
		return (ARCHIVE_FATAL);
	}
	memset(mine, 0, sizeof(*mine));
	mine->buff = buff;
	mine->size = buffSize;
	mine->client_size = used;
	return (archive_write_open(a, mine,
		    memory_write_open, memory_write, memory_write_close));
}

static int
memory_write_open(struct archive *a, void *client_data)
{
	struct write_memory_data *mine;
	mine = client_data;
	mine->used = 0;
	if (mine->client_size != NULL)
		*mine->client_size = mine->used;
	/* Disable padding if it hasn't been set explicitly. */
	if (-1 == archive_write_get_bytes_in_last_block(a))
		archive_write_set_bytes_in_last_block(a, 1);
	return (ARCHIVE_OK);
}

/*
 * Copy the data into the client buffer.
 * Note that we update mine->client_size on every write.
 * In particular, this means the client can follow exactly
 * how much has been written into their buffer at any time.
 */
static ssize_t
memory_write(struct archive *a, void *client_data, const void *buff, size_t length)
{
	struct write_memory_data *mine;
	mine = client_data;

	if (mine->used + length > mine->size) {
		archive_set_error(a, ENOMEM, "Buffer exhausted");
		return (ARCHIVE_FATAL);
	}
	memcpy(mine->buff + mine->used, buff, length);
	mine->used += length;
	if (mine->client_size != NULL)
		*mine->client_size = mine->used;
	return (length);
}

static int
memory_write_close(struct archive *a, void *client_data)
{
	struct write_memory_data *mine;
	(void)a; /* UNUSED */
	mine = client_data;
	free(mine);
	return (ARCHIVE_OK);
}