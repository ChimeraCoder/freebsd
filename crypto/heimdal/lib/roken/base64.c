
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

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "base64.h"

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int
pos(char c)
{
    const char *p;
    for (p = base64_chars; *p; p++)
	if (*p == c)
	    return p - base64_chars;
    return -1;
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
base64_encode(const void *data, int size, char **str)
{
    char *s, *p;
    int i;
    int c;
    const unsigned char *q;

    if (size > INT_MAX/4 || size < 0) {
	*str = NULL;
	return -1;
    }

    p = s = (char *) malloc(size * 4 / 3 + 4);
    if (p == NULL) {
        *str = NULL;
	return -1;
    }
    q = (const unsigned char *) data;

    for (i = 0; i < size;) {
	c = q[i++];
	c *= 256;
	if (i < size)
	    c += q[i];
	i++;
	c *= 256;
	if (i < size)
	    c += q[i];
	i++;
	p[0] = base64_chars[(c & 0x00fc0000) >> 18];
	p[1] = base64_chars[(c & 0x0003f000) >> 12];
	p[2] = base64_chars[(c & 0x00000fc0) >> 6];
	p[3] = base64_chars[(c & 0x0000003f) >> 0];
	if (i > size)
	    p[3] = '=';
	if (i > size + 1)
	    p[2] = '=';
	p += 4;
    }
    *p = 0;
    *str = s;
    return (int) strlen(s);
}

#define DECODE_ERROR 0xffffffff

static unsigned int
token_decode(const char *token)
{
    int i;
    unsigned int val = 0;
    int marker = 0;
    if (strlen(token) < 4)
	return DECODE_ERROR;
    for (i = 0; i < 4; i++) {
	val *= 64;
	if (token[i] == '=')
	    marker++;
	else if (marker > 0)
	    return DECODE_ERROR;
	else
	    val += pos(token[i]);
    }
    if (marker > 2)
	return DECODE_ERROR;
    return (marker << 24) | val;
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
base64_decode(const char *str, void *data)
{
    const char *p;
    unsigned char *q;

    q = data;
    for (p = str; *p && (*p == '=' || strchr(base64_chars, *p)); p += 4) {
	unsigned int val = token_decode(p);
	unsigned int marker = (val >> 24) & 0xff;
	if (val == DECODE_ERROR)
	    return -1;
	*q++ = (val >> 16) & 0xff;
	if (marker < 2)
	    *q++ = (val >> 8) & 0xff;
	if (marker < 1)
	    *q++ = val & 0xff;
    }
    return q - (unsigned char *) data;
}