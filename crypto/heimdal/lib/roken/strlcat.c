
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

#ifndef HAVE_STRLCAT

ROKEN_LIB_FUNCTION size_t ROKEN_LIB_CALL
strlcat (char *dst, const char *src, size_t dst_sz)
{
    size_t len;
#if defined(_MSC_VER) && _MSC_VER >= 1400
    len = strnlen_s(dst, dst_sz);
#elif defined(HAVE_STRNLEN)
    len = strnlen(dst, dst_sz);
#else
    len = strlen(dst);
#endif

    if (dst_sz <= len)
	/* the total size of dst is less than the string it contains;
           this could be considered bad input, but we might as well
           handle it */
	return len + strlen(src);

    return len + strlcpy (dst + len, src, dst_sz - len);
}

#endif