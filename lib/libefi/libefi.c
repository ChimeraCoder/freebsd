
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libefi_int.h"

static int __iofd = -1;

static void
iodev_fd_close(void)
{

	close(__iofd);
}

static int
iodev_fd(int *fd)
{

	*fd = __iofd;
	if (__iofd != -1)
		return (0);

	__iofd = open("/dev/io", O_RDWR);
	if (__iofd == -1)
		return (errno);

	atexit(iodev_fd_close);
	*fd = __iofd;
	return (0);
}

int
libefi_ucs2_to_utf8(u_short *nm, size_t *szp, char *name)
{
	size_t len, sz;
	u_short c;

	len = 0;
	sz = *szp;
	while (*nm) {
		c = *nm++;
		if (c > 0x7ff) {
			if (len++ < sz)
				*name++ = 0xE0 | (c >> 12);
			if (len++ < sz)
				*name++ = 0x80 | ((c >> 6) & 0x3f);
			if (len++ < sz)
				*name++ = 0x80 | (c & 0x3f);
		} else if (c > 0x7f) {
			if (len++ < sz)
				*name++ = 0xC0 | ((c >> 6) & 0x1f);
			if (len++ < sz)
				*name++ = 0x80 | (c & 0x3f);
		} else {
			if (len++ < sz)
				*name++ = (c & 0x7f);
		}
	}
	if (len++ < sz)
		*name++ = 0;

	*szp = len;
	return ((len <= sz) ? 0 : EOVERFLOW);
}

int
libefi_utf8_to_ucs2(char *name, size_t *szp, u_short **nmp)
{
	u_short *nm;
	size_t sz;
	uint32_t ucs4;
	int c, bytes;

	*szp = sz = (*szp == 0) ? strlen(name) * 2 + 2 : *szp;
	*nmp = nm = malloc(sz);

	ucs4 = 0;
	bytes = 0;
	while (sz > 1 && *name != '\0') {
		c = *name++;
		/*
		 * Conditionalize on the two major character types:
		 * initial and followup characters.
		 */
		if ((c & 0xc0) != 0x80) {
			/* Initial characters. */
			if (bytes != 0) {
				free(nm);
				return (EILSEQ);
			}
			if ((c & 0xf8) == 0xf0) {
				ucs4 = c & 0x07;
				bytes = 3;
			} else if ((c & 0xf0) == 0xe0) {
				ucs4 = c & 0x0f;
				bytes = 2;
			} else if ((c & 0xe0) == 0xc0) {
				ucs4 = c & 0x1f;
				bytes = 1;
			} else {
				ucs4 = c & 0x7f;
				bytes = 0;
			}
		} else {
			/* Followup characters. */
			if (bytes > 0) {
				ucs4 = (ucs4 << 6) + (c & 0x3f);
				bytes--;
			} else if (bytes == 0) {
				free(nm);
				return (EILSEQ);
			}
		}
		if (bytes == 0) {
			if (ucs4 > 0xffff) {
				free(nm);
				return (EILSEQ);
			}
			*nm++ = (u_short)ucs4;
			sz -= 2;
		}
	}
	if (sz < 2) {
		free(nm);
		return (EDOOFUS);
	}
	*nm = 0;
	return (0);
}

int
libefi_efivar(struct iodev_efivar_req *req)
{
	int error, fd;

	error = iodev_fd(&fd);
	if (!error)
		error = (ioctl(fd, IODEV_EFIVAR, req) == -1) ? errno : 0;
	if (!error)
		error = req->result;
	return (error);
}