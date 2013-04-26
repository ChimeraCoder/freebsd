
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
#include "roken.h"
#include <ctype.h>
#include "hex.h"

static const char hexchar[16] = "0123456789ABCDEF";

static int
pos(char c)
{
    const char *p;
    c = toupper((unsigned char)c);
    for (p = hexchar; *p; p++)
	if (*p == c)
	    return p - hexchar;
    return -1;
}

ROKEN_LIB_FUNCTION ssize_t ROKEN_LIB_CALL
hex_encode(const void *data, size_t size, char **str)
{
    const unsigned char *q = data;
    size_t i;
    char *p;

    /* check for overflow */
    if (size * 2 < size) {
        *str = NULL;
	return -1;
    }

    p = malloc(size * 2 + 1);
    if (p == NULL) {
        *str = NULL;
	return -1;
    }

    for (i = 0; i < size; i++) {
	p[i * 2] = hexchar[(*q >> 4) & 0xf];
	p[i * 2 + 1] = hexchar[*q & 0xf];
	q++;
    }
    p[i * 2] = '\0';
    *str = p;

    return i * 2;
}

ROKEN_LIB_FUNCTION ssize_t ROKEN_LIB_CALL
hex_decode(const char *str, void *data, size_t len)
{
    size_t l;
    unsigned char *p = data;
    size_t i;

    l = strlen(str);

    /* check for overflow, same as (l+1)/2 but overflow safe */
    if ((l/2) + (l&1) > len)
	return -1;

    if (l & 1) {
	p[0] = pos(str[0]);
	str++;
	p++;
    }
    for (i = 0; i < l / 2; i++)
	p[i] = pos(str[i * 2]) << 4 | pos(str[(i * 2) + 1]);
    return i + (l & 1);
}