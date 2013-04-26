
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
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
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

struct write_FILE_data {
	FILE		*f;
};

static int	file_close(struct archive *, void *);
static int	file_open(struct archive *, void *);
static ssize_t	file_write(struct archive *, void *, const void *buff, size_t);

int
archive_write_open_FILE(struct archive *a, FILE *f)
{
	struct write_FILE_data *mine;

	mine = (struct write_FILE_data *)malloc(sizeof(*mine));
	if (mine == NULL) {
		archive_set_error(a, ENOMEM, "No memory");
		return (ARCHIVE_FATAL);
	}
	mine->f = f;
	return (archive_write_open(a, mine,
		    file_open, file_write, file_close));
}

static int
file_open(struct archive *a, void *client_data)
{
	(void)a; /* UNUSED */
	(void)client_data; /* UNUSED */

	return (ARCHIVE_OK);
}

static ssize_t
file_write(struct archive *a, void *client_data, const void *buff, size_t length)
{
	struct write_FILE_data	*mine;
	size_t	bytesWritten;

	mine = client_data;
	for (;;) {
		bytesWritten = fwrite(buff, 1, length, mine->f);
		if (bytesWritten <= 0) {
			if (errno == EINTR)
				continue;
			archive_set_error(a, errno, "Write error");
			return (-1);
		}
		return (bytesWritten);
	}
}

static int
file_close(struct archive *a, void *client_data)
{
	struct write_FILE_data	*mine = client_data;

	(void)a; /* UNUSED */
	free(mine);
	return (ARCHIVE_OK);
}