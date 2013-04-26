
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

#include "includes.h"

#include <sys/types.h>
#include <sys/uio.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "buffer.h"
#include "log.h"
#include "atomicio.h"
#include "msg.h"
#include "misc.h"

int
ssh_msg_send(int fd, u_char type, Buffer *m)
{
	u_char buf[5];
	u_int mlen = buffer_len(m);

	debug3("ssh_msg_send: type %u", (unsigned int)type & 0xff);

	put_u32(buf, mlen + 1);
	buf[4] = type;		/* 1st byte of payload is mesg-type */
	if (atomicio(vwrite, fd, buf, sizeof(buf)) != sizeof(buf)) {
		error("ssh_msg_send: write");
		return (-1);
	}
	if (atomicio(vwrite, fd, buffer_ptr(m), mlen) != mlen) {
		error("ssh_msg_send: write");
		return (-1);
	}
	return (0);
}

int
ssh_msg_recv(int fd, Buffer *m)
{
	u_char buf[4];
	u_int msg_len;

	debug3("ssh_msg_recv entering");

	if (atomicio(read, fd, buf, sizeof(buf)) != sizeof(buf)) {
		if (errno != EPIPE)
			error("ssh_msg_recv: read: header");
		return (-1);
	}
	msg_len = get_u32(buf);
	if (msg_len > 256 * 1024) {
		error("ssh_msg_recv: read: bad msg_len %u", msg_len);
		return (-1);
	}
	buffer_clear(m);
	buffer_append_space(m, msg_len);
	if (atomicio(read, fd, buffer_ptr(m), msg_len) != msg_len) {
		error("ssh_msg_recv: read: %s", strerror(errno));
		return (-1);
	}
	return (0);
}