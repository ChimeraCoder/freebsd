
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netnatm/unimsg.h>

/* the amount of extra bytes to allocate */
#define EXTRA	128

/*
 * Allocate a message that can hold at least 's' bytes. Return NULL if
 * allocation fails.
 */
struct uni_msg *
uni_msg_alloc(size_t s)
{
	struct uni_msg *m;

	s += EXTRA;

	if ((m = malloc(sizeof(struct uni_msg))) == NULL)
		return NULL;
	if ((m->b_buf = malloc(s)) == NULL) {
		free(m);
		return (NULL);
	}
	m->b_rptr = m->b_wptr = m->b_buf;
	m->b_lim = m->b_buf + s;
	return (m);
}

/*
 * Destroy the message and free memory
 */
void
uni_msg_destroy(struct uni_msg *m)
{
	free(m->b_buf);
	free(m);
}

/*
 * Extend message by at least 's' additional bytes.
 * May reallocate the message buffer. Return -1 on errors, 0 if ok.
 * If an error occurs the message is destroyed.
 */
int
uni_msg_extend(struct uni_msg *m, size_t s)
{
	u_char *b;
	size_t len, leading, newsize;

	len = uni_msg_len(m);
	newsize = m->b_wptr - m->b_buf + s + EXTRA;
	leading = m->b_rptr - m->b_buf;
	if ((b = realloc(m->b_buf, newsize)) == NULL) {
		free(m->b_buf);
		free(m);
		return (-1);
	}
	m->b_buf = b;
	m->b_rptr = m->b_buf + leading;
	m->b_wptr = m->b_rptr + len;
	m->b_lim = m->b_buf + newsize;

	return (0);
}

/*
 * Append the given buffer to the message. May reallocate the message
 * buffer. Return 0 if ok, -1 on errors.
 */
int
uni_msg_append(struct uni_msg *m, void *buf, size_t size)
{
	int error;

	if ((error = uni_msg_ensure(m, size)))
		return (error);
	memcpy(m->b_wptr, buf, size);
	m->b_wptr += size;

	return (0);
}

/*
 * Construct a message from a number of pieces. The list of pieces must end
 * with a NULL pointer.
 */
struct uni_msg *
uni_msg_build(void *ptr, ...)
{
	va_list ap;
	struct uni_msg *m;
	size_t len, n;
	void *p1;

	len = 0;
	va_start(ap, ptr);
	p1 = ptr;
	while (p1 != NULL) {
		n = va_arg(ap, size_t);
		len += n;
		p1 = va_arg(ap, void *);
	}
	va_end(ap);

	if ((m = uni_msg_alloc(len)) == NULL)
		return (NULL);

	va_start(ap, ptr);
	p1 = ptr;
	while (p1 != NULL) {
		n = va_arg(ap, size_t);
		memcpy(m->b_wptr, p1, n);
		m->b_wptr += n;
		p1 = va_arg(ap, void *);
	}
	va_end(ap);

	return (m);
}

/*
 * Strip the last 32 bit word from the buffer.
 * Barf if there is no word left.
 */
u_int
uni_msg_strip32(struct uni_msg *msg)
{
	uint32_t w;

	msg->b_wptr -= 4;
	bcopy(msg->b_wptr, &w, 4);
	return (ntohl(w));
}

/*
 * Strip the first four bytes of the buffer.
 */
u_int
uni_msg_get32(struct uni_msg *msg)
{
	uint32_t w;

	bcopy(msg->b_rptr, &w, 4);
	msg->b_rptr += 4;
	return (ntohl(w));
}

/*
 * Append a 32 bit word to the buffer.
 */
int
uni_msg_append32(struct uni_msg *msg, u_int u)
{
	if (uni_msg_ensure(msg, 4) == -1)
		return (-1);
	u = htonl(u);
	bcopy(&u, msg->b_wptr, 4);
	msg->b_wptr += 4;
	return (0);
}

/*
 * Append a byte to the buffer.
 */
int
uni_msg_append8(struct uni_msg *msg, u_int u)
{
	if (uni_msg_ensure(msg, 1) == -1)
		return (-1);
	*msg->b_wptr++ = u;
	return (0);
}

/*
 * Return the i-th word counted from the end of the buffer.
 * i=-1 will return the last 32bit word, i=-2 the 2nd last.
 * Assumes that the word is in the buffer.
 */
u_int
uni_msg_trail32(const struct uni_msg *msg, int i)
{
	u_int w;

	bcopy(msg->b_wptr + 4 * i, &w, 4);
	return (ntohl(w));
}


/*
 * Make a duplicate.
 */
struct uni_msg *
uni_msg_dup(const struct uni_msg *inp)
{
	struct uni_msg *msg;
	u_int len, off;

	len = inp->b_wptr - inp->b_rptr;
	off = inp->b_rptr - inp->b_buf;
	if ((msg = uni_msg_alloc(inp->b_lim - inp->b_buf)) == NULL)
		return (NULL);
	msg->b_rptr = msg->b_buf + off;
	msg->b_wptr = msg->b_rptr + len;
	(void)memcpy(msg->b_rptr, inp->b_rptr, len);

	return (msg);
}