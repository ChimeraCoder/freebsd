
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
 * caltontp - convert a date to an NTP time
 */#include <sys/types.h>

#include "ntp_types.h"
#include "ntp_calendar.h"
#include "ntp_stdlib.h"

u_long
caltontp(
	register const struct calendar *jt
	)
{
    u_long ace_days;			     /* absolute Christian Era days */
    u_long ntp_days;
    int    prior_years;
    u_long ntp_time;
    
    /*
     * First convert today's date to absolute days past 12/1/1 BC
     */
    prior_years = jt->year-1;
    ace_days = jt->yearday		     /* days this year */
	+(DAYSPERYEAR*prior_years)	     /* plus days in previous years */
	+(prior_years/4)		     /* plus prior years's leap days */
	-(prior_years/100)		     /* minus leapless century years */
	+(prior_years/400);		     /* plus leapful Gregorian yrs */

    /*
     * Subtract out 1/1/1900, the beginning of the NTP epoch
     */
    ntp_days = ace_days - DAY_NTP_STARTS;

    /*
     * Do the obvious:
     */
    ntp_time = 
	ntp_days*SECSPERDAY+SECSPERMIN*(MINSPERHR*jt->hour + jt->minute);

    return ntp_time;
}