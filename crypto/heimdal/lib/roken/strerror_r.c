
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

#if (!defined(HAVE_STRERROR_R) && !defined(strerror_r)) || (!defined(STRERROR_R_PROTO_COMPATIBLE) && defined(HAVE_STRERROR_R))

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "roken.h"

#ifdef _MSC_VER

int ROKEN_LIB_FUNCTION
rk_strerror_r(int eno, char * strerrbuf, size_t buflen)
{
    errno_t err;

    err = strerror_s(strerrbuf, buflen, eno);
    if (err != 0) {
        int code;
        code = sprintf_s(strerrbuf, buflen, "Error % occurred.", eno);
        err = ((code != 0)? errno : 0);
    }

    return err;
}

#else  /* _MSC_VER */

int ROKEN_LIB_FUNCTION
rk_strerror_r(int eno, char *strerrbuf, size_t buflen)
{
    /* Assume is the linux broken strerror_r (returns the a buffer (char *) if the input buffer wasn't use */
#ifdef HAVE_STRERROR_R
    const char *str;
    str = strerror_r(eno, strerrbuf, buflen);
    if (str != strerrbuf)
	if (strlcpy(strerrbuf, str, buflen) >= buflen)
	    return ERANGE;
    return 0;
#else
    int ret;
    ret = strlcpy(strerrbuf, strerror(eno), buflen);
    if (ret > buflen)
	return ERANGE;
    return 0;
#endif
}

#endif  /* !_MSC_VER */

#endif