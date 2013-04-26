
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

#include <der_locl.h>

RCSID("$Id$");

static int
test_timegm(void)
{
    int ret = 0;
    struct tm tm;
    time_t t;

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 106;
    tm.tm_mon = 9;
    tm.tm_mday = 1;
    tm.tm_hour = 10;
    tm.tm_min = 3;

    t = _der_timegm(&tm);
    if (t != 1159696980)
	ret += 1;

    tm.tm_mday = 0;
    t = _der_timegm(&tm);
    if (t != -1)
	ret += 1;

    _der_gmtime(1159696980, &tm);
    if (tm.tm_year != 106 ||
	tm.tm_mon != 9 ||
	tm.tm_mday != 1 ||
	tm.tm_hour != 10 ||
	tm.tm_min != 3 ||
	tm.tm_sec != 0)
      errx(1, "tmtime failes");

    return ret;
}

int
main(int argc, char **argv)
{
    int ret = 0;

    ret += test_timegm();

    return ret;
}