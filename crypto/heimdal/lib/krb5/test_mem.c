
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

/*
 * Test run functions, to be used with valgrind to detect memoryleaks.
 */

static void
check_log(void)
{
    int i;

    for (i = 0; i < 10; i++) {
	krb5_log_facility *logfacility;
	krb5_context context;
	krb5_error_code ret;

	ret = krb5_init_context(&context);
	if (ret)
	    errx (1, "krb5_init_context failed: %d", ret);

	krb5_initlog(context, "test-mem", &logfacility);
	krb5_addlog_dest(context, logfacility, "0/STDERR:");
	krb5_set_warn_dest(context, logfacility);

	krb5_free_context(context);
    }
}


int
main(int argc, char **argv)
{
    setprogname(argv[0]);

    check_log();

    return 0;
}