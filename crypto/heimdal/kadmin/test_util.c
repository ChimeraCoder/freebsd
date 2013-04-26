
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

#include "kadmin_locl.h"

krb5_context context;
void *kadm_handle;

struct {
    const char *str;
    int ret;
    time_t t;
} ts[] = {
    { "2006-12-22 18:09:00", 0, 1166810940 },
    { "2006-12-22", 0, 1166831999 },
    { "2006-12-22 23:59:59", 0, 1166831999 }
};

static int
test_time(void)
{
    int i, errors = 0;

    for (i = 0; i < sizeof(ts)/sizeof(ts[0]); i++) {
	time_t t;
	int ret;

	ret = str2time_t (ts[i].str, &t);
	if (ret != ts[i].ret) {
	    printf("%d: %d is wrong ret\n", i, ret);
	    errors++;
	}
	else if (t != ts[i].t) {
	    printf("%d: %d is wrong time\n", i, (int)t);
	    errors++;
	}
    }

    return errors;
}


int
main(int argc, char **argv)
{
    krb5_error_code ret;

    setprogname(argv[0]);

    ret = krb5_init_context(&context);
    if (ret)
	errx (1, "krb5_init_context failed: %d", ret);

    ret = 0;
    ret += test_time();

    krb5_free_context(context);

    return ret;
}