
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
#ifndef HAVE_GETTIMEOFDAY

#ifdef _WIN32

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
gettimeofday (struct timeval *tp, void *ignore)
{
    FILETIME ft;
    ULARGE_INTEGER li;
    ULONGLONG ull;

    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    ull = li.QuadPart;

    ull -= 116444736000000000i64;
    ull /= 10i64;               /* ull is now in microseconds */

    tp->tv_usec = (ull % 1000000i64);
    tp->tv_sec  = (ull / 1000000i64);

    return 0;
}

#else

/*
 * Simple gettimeofday that only returns seconds.
 */
ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
gettimeofday (struct timeval *tp, void *ignore)
{
     time_t t;

     t = time(NULL);
     tp->tv_sec  = (long) t;
     tp->tv_usec = 0;
     return 0;
}

#endif  /* !_WIN32 */
#endif