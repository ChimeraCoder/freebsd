
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

#include <stdio.h>
#include <time.h>
#include "roken.h"

#ifndef HAVE_LOCALTIME_R

ROKEN_LIB_FUNCTION struct tm * ROKEN_LIB_CALL
localtime_r(const time_t *timer, struct tm *result)
{
#ifdef _MSC_VER

    return (localtime_s(result, timer) == 0)? result : NULL;

#else
    struct tm *tm;

    tm = localtime((time_t *)timer);
    if (tm == NULL)
	return NULL;
    *result = *tm;
    return result;
#endif
}

#endif