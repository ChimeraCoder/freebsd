
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
/*
 * calyearstart - determine the NTP time at midnight of January 1 in
 *		  the year of the given date.
 */#include <sys/types.h>

#include "ntp_types.h"
#include "ntp_calendar.h"
#include "ntp_stdlib.h"

u_long
calyearstart(u_long ntp_time)
{
    struct calendar jt;

    caljulian(ntp_time,&jt);
    jt.yearday  = 1;
    jt.monthday = 1;
    jt.month    = 1;
    jt.hour = jt.minute = jt.second = 0;
    return caltontp(&jt);
}