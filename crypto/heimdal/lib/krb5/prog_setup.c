
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
#include <getarg.h>
#include <err.h>

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_std_usage(int code, struct getargs *args, int num_args)
{
    arg_printusage(args, num_args, NULL, "");
    exit(code);
}

KRB5_LIB_FUNCTION int KRB5_LIB_CALL
krb5_program_setup(krb5_context *context, int argc, char **argv,
		   struct getargs *args, int num_args,
		   void (KRB5_LIB_CALL *usage)(int, struct getargs*, int))
{
    krb5_error_code ret;
    int optidx = 0;

    if(usage == NULL)
	usage = krb5_std_usage;

    setprogname(argv[0]);
    ret = krb5_init_context(context);
    if (ret)
	errx (1, "krb5_init_context failed: %d", ret);

    if(getarg(args, num_args, argc, argv, &optidx))
	(*usage)(1, args, num_args);
    return optidx;
}