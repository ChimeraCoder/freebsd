
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

static int
is_leap(unsigned y)
{
    y += 1900;
    return (y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0);
}

/*
 * XXX This is a simplifed version of timegm, it needs to support out of
 * bounds values.
 */

time_t
rk_timegm (struct tm *tm)
{
  static const unsigned ndays[2][12] ={
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
  time_t res = 0;
  unsigned i;

  if (tm->tm_year < 0)
      return -1;
  if (tm->tm_mon < 0 || tm->tm_mon > 11)
      return -1;
  if (tm->tm_mday < 1 || tm->tm_mday > ndays[is_leap(tm->tm_year)][tm->tm_mon])
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