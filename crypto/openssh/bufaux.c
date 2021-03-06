
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

#include <openssl/bn.h>

#include <string.h>
#include <stdarg.h>

#include "xmalloc.h"
#include "buffer.h"
#include "log.h"
#include "misc.h"

/*
 * Returns integers from the buffer (msb first).
 */

int
buffer_get_short_ret(u_short *ret, Buffer *buffer)
{
	u_char buf[2];

	if (buffer_get_ret(buffer, (char *) buf, 2) == -1)
		return (-1);
	*ret = get_u16(buf);
	return (0);
}

u_short
buffer_get_short(Buffer *buffer)
{
	u_short ret;

	if (buffer_get_short_ret(&ret, buffer) == -1)
		fatal("buffer_get_short: buffer error");

	return (ret);
}

int
buffer_get_int_ret(u_int *ret, Buffer *buffer)
{
	u_char buf[4];

	if (buffer_get_ret(buffer, (char *) buf, 4) == -1)
		return (-1);
	if (ret != NULL)
		*ret = get_u32(buf);
	return (0);
}

u_int
buffer_get_int(Buffer *buffer)
{
	u_int ret;

	if (buffer_get_int_ret(&ret, buffer) == -1)
		fatal("buffer_get_int: buffer error");

	return (ret);
}

int
buffer_get_int64_ret(u_int64_t *ret, Buffer *buffer)
{
	u_char buf[8];

	if (buffer_get_ret(buffer, (char *) buf, 8) == -1)
		return (-1);
	if (ret != NULL)
		*ret = get_u64(buf);
	return (0);
}

u_int64_t
buffer_get_int64(Buffer *buffer)
{
	u_int64_t ret;

	if (buffer_get_int64_ret(&ret, buffer) == -1)
		fatal("buffer_get_int: buffer error");

	return (ret);
}

/*
 * Stores integers in the buffer, msb first.
 */
void
buffer_put_short(Buffer *buffer, u_short value)
{
	char buf[2];

	put_u16(buf, value);
	buffer_append(buffer, buf, 2);
}

void
buffer_put_int(Buffer *buffer, u_int value)
{
	char buf[4];

	put_u32(buf, value);
	buffer_append(buffer, buf, 4);
}

void
buffer_put_int64(Buffer *buffer, u_int64_t value)
{
	char buf[8];

	put_u64(buf, value);
	buffer_append(buffer, buf, 8);
}

/*
 * Returns an arbitrary binary string from the buffer.  The string cannot
 * be longer than 256k.  The returned value points to memory allocated
 * with xmalloc; it is the responsibility of the calling function to free
 * the data.  If length_ptr is non-NULL, the length of the returned data
 * will be stored there.  A null character will be automatically appended
 * to the returned string, and is not counted in length.
 */
void *
buffer_get_string_ret(Buffer *buffer, u_int *length_ptr)
{
	u_char *value;
	u_int len;

	/* Get the length. */
	if (buffer_get_int_ret(&len, buffer) != 0) {
		error("buffer_get_string_ret: cannot extract length");
		return (NULL);
	}
	if (len > 256 * 1024) {
		error("buffer_get_string_ret: bad string length %u", len);
		return (NULL);
	}
	/* Allocate space for the string.  Add one byte for a null character. */
	value = xmalloc(len + 1);
	/* Get the string. */
	if (buffer_get_ret(buffer, value, len) == -1) {
		error("buffer_get_string_ret: buffer_get failed");
		xfree(value);
		return (NULL);
	}
	/* Append a null character to make processing easier. */
	value[len] = '\0';
	/* Optionally return the length of the string. */
	if (length_ptr)
		*length_ptr = len;
	return (value);
}

void *
buffer_get_string(Buffer *buffer, u_int *length_ptr)
{
	void *ret;

	if ((ret = buffer_get_string_ret(buffer, length_ptr)) == NULL)
		fatal("buffer_get_string: buffer error");
	return (ret);
}

char *
buffer_get_cstring_ret(Buffer *buffer, u_int *length_ptr)
{
	u_int length;
	char *cp, *ret = buffer_get_string_ret(buffer, &length);

	if (ret == NULL)
		return NULL;
	if ((cp = memchr(ret, '\0', length)) != NULL) {
		/* XXX allow \0 at end-of-string for a while, remove later */
		if (cp == ret + length - 1)
			error("buffer_get_cstring_ret: string contains \\0");
		else {
			bzero(ret, length);
			xfree(ret);
			return NULL;
		}
	}
	if (length_ptr != NULL)
		*length_ptr = length;
	return ret;
}

char *
buffer_get_cstring(Buffer *buffer, u_int *length_ptr)
{
	char *ret;

	if ((ret = buffer_get_cstring_ret(buffer, length_ptr)) == NULL)
		fatal("buffer_get_cstring: buffer error");
	return ret;
}

void *
buffer_get_string_ptr_ret(Buffer *buffer, u_int *length_ptr)
{
	void *ptr;
	u_int len;

	if (buffer_get_int_ret(&len, buffer) != 0)
		return NULL;
	if (len > 256 * 1024) {
		error("buffer_get_string_ptr: bad string length %u", len);
		return NULL;
	}
	ptr = buffer_ptr(buffer);
	buffer_consume(buffer, len);
	if (length_ptr)
		*length_ptr = len;
	return (ptr);
}

void *
buffer_get_string_ptr(Buffer *buffer, u_int *length_ptr)
{
	void *ret;

	if ((ret = buffer_get_string_ptr_ret(buffer, length_ptr)) == NULL)
		fatal("buffer_get_string_ptr: buffer error");
	return (ret);
}

/*
 * Stores and arbitrary binary string in the buffer.
 */
void
buffer_put_string(Buffer *buffer, const void *buf, u_int len)
{
	buffer_put_int(buffer, len);
	buffer_append(buffer, buf, len);
}
void
buffer_put_cstring(Buffer *buffer, const char *s)
{
	if (s == NULL)
		fatal("buffer_put_cstring: s == NULL");
	buffer_put_string(buffer, s, strlen(s));
}

/*
 * Returns a character from the buffer (0 - 255).
 */
int
buffer_get_char_ret(char *ret, Buffer *buffer)
{
	if (buffer_get_ret(buffer, ret, 1) == -1) {
		error("buffer_get_char_ret: buffer_get_ret failed");
		return (-1);
	}
	return (0);
}

int
buffer_get_char(Buffer *buffer)
{
	char ch;

	if (buffer_get_char_ret(&ch, buffer) == -1)
		fatal("buffer_get_char: buffer error");
	return (u_char) ch;
}

/*
 * Stores a character in the buffer.
 */
void
buffer_put_char(Buffer *buffer, int value)
{
	char ch = value;

	buffer_append(buffer, &ch, 1);
}