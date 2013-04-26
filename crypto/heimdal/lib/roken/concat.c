
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

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
roken_concat (char *s, size_t len, ...)
{
    int ret;
    va_list args;

    va_start(args, len);
    ret = roken_vconcat (s, len, args);
    va_end(args);
    return ret;
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
roken_vconcat (char *s, size_t len, va_list args)
{
    const char *a;

    while ((a = va_arg(args, const char*))) {
	size_t n = strlen (a);

	if (n >= len)
	    return -1;
	memcpy (s, a, n);
	s += n;
	len -= n;
    }
    *s = '\0';
    return 0;
}

ROKEN_LIB_FUNCTION size_t ROKEN_LIB_CALL
roken_vmconcat (char **s, size_t max_len, va_list args)
{
    const char *a;
    char *p, *q;
    size_t len = 0;
    *s = NULL;
    p = malloc(1);
    if(p == NULL)
	return 0;
    len = 1;
    while ((a = va_arg(args, const char*))) {
	size_t n = strlen (a);

	if(max_len && len + n > max_len){
	    free(p);
	    return 0;
	}
	q = realloc(p, len + n);
	if(q == NULL){
	    free(p);
	    return 0;
	}
	p = q;
	memcpy (p + len - 1, a, n);
	len += n;
    }
    p[len - 1] = '\0';
    *s = p;
    return len;
}

ROKEN_LIB_FUNCTION size_t ROKEN_LIB_CALL
roken_mconcat (char **s, size_t max_len, ...)
{
    size_t ret;
    va_list args;

    va_start(args, max_len);
    ret = roken_vmconcat (s, max_len, args);
    va_end(args);
    return ret;
}