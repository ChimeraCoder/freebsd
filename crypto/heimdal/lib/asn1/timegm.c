
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

#include "der_locl.h"

#define ASN1_MAX_YEAR	2000

static int
is_leap(unsigned y)
{
    y += 1900;
    return (y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0);
}

static const unsigned ndays[2][12] ={
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

/*
 * This is a simplifed version of timegm(3) that doesn't accept out of
 * bound values that timegm(3) normally accepts but those are not
 * valid in asn1 encodings.
 */

time_t
_der_timegm (struct tm *tm)
{
  time_t res = 0;
  int i;

  /*
   * See comment in _der_gmtime
   */
  if (tm->tm_year > ASN1_MAX_YEAR)
      return 0;

  if (tm->tm_year < 0)
      return -1;
  if (tm->tm_mon < 0 || tm->tm_mon > 11)
      return -1;
  if (tm->tm_mday < 1 || tm->tm_mday > (int)ndays[is_leap(tm->tm_year)][tm->tm_mon])
      return -1;
  if (tm->tm_hour < 0 || tm->tm_hour > 23)
      return -1;
  if (tm->tm_min < 0 || tm->tm_min > 59)
      return -1;
  if (tm->tm_sec < 0 || tm->tm_sec > 59)
      return -1;

  for (i = 70; i < tm->tm_year; ++i)
    res += is_leap(i) ? 366 : 365;

  for (i = 0; i < tm->tm_mon; ++i)
    res += ndays[is_leap(tm->tm_year)][i];
  res += tm->tm_mday - 1;
  res *= 24;
  res += tm->tm_hour;
  res *= 60;
  res += tm->tm_min;
  res *= 60;
  res += tm->tm_sec;
  return res;
}

struct tm *
_der_gmtime(time_t t, struct tm *tm)
{
    time_t secday = t % (3600 * 24);
    time_t days = t / (3600 * 24);

    memset(tm, 0, sizeof(*tm));

    tm->tm_sec = secday % 60;
    tm->tm_min = (secday % 3600) / 60;
    tm->tm_hour = secday / 3600;

    /*
     * Refuse to calculate time ~ 2000 years into the future, this is
     * not possible for systems where time_t is a int32_t, however,
     * when time_t is a int64_t, that can happen, and this becomes a
     * denial of sevice.
     */
    if (days > (ASN1_MAX_YEAR * 365))
	return NULL;

    tm->tm_year = 70;
    while(1) {
	unsigned dayinyear = (is_leap(tm->tm_year) ? 366 : 365);
	if (days < dayinyear)
	    break;
	tm->tm_year += 1;
	days -= dayinyear;
    }
    tm->tm_mon = 0;

    while (1) {
	unsigned daysinmonth = ndays[is_leap(tm->tm_year)][tm->tm_mon];
	if (days < daysinmonth)
	    break;
	days -= daysinmonth;
	tm->tm_mon++;
    }
    tm->tm_mday = days + 1;

    return tm;
}