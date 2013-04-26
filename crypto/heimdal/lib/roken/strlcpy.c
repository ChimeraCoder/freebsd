
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

#ifndef HAVE_STRLCPY

#if defined(_MSC_VER) && _MSC_VER >= 1400

ROKEN_LIB_FUNCTION size_t ROKEN_LIB_CALL
strlcpy (char *dst, const char *src, size_t dst_cch)
{
    errno_t e;

    if (dst_cch > 0)
        e = strncpy_s(dst, dst_cch, src, _TRUNCATE);

    return strlen (src);
}

#else

ROKEN_LIB_FUNCTION size_t ROKEN_LIB_CALL
strlcpy (char *dst, const char *src, size_t dst_sz)
{
    size_t n;

    for (n = 0; n < dst_sz; n++) {
	if ((*dst++ = *src++) == '\0')
	    break;
    }

    if (n < dst_sz)
	return n;
    if (n > 0)
	*(dst - 1) = '\0';
    return n + strlen (src);
}

#endif

#endif