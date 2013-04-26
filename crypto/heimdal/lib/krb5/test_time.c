
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

#include "krb5_locl.h"
#include <err.h>

static void
check_set_time(krb5_context context)
{
    krb5_error_code ret;
    krb5_timestamp sec;
    int32_t usec;
    struct timeval tv;
    int diff = 10;
    int diff2;

    gettimeofday(&tv, NULL);

    ret = krb5_set_real_time(context, tv.tv_sec + diff, tv.tv_usec);
    if (ret)
	krb5_err(context, 1, ret, "krb5_us_timeofday");

    ret = krb5_us_timeofday(context, &sec, &usec);
    if (ret)
	krb5_err(context, 1, ret, "krb5_us_timeofday");

    diff2 = abs(sec - tv.tv_sec);

    if (diff2 < 9 || diff > 11)
	krb5_errx(context, 1, "set time error: diff: %d",
		  abs(sec - tv.tv_sec));
}



int
main(int argc, char **argv)
{
    krb5_context context;
    krb5_error_code ret;

    ret = krb5_init_context(&context);
    if (ret)
	errx(1, "krb5_init_context %d", ret);

    check_set_time(context);
    check_set_time(context);
    check_set_time(context);
    check_set_time(context);
    check_set_time(context);

    krb5_free_context(context);

    return 0;
}