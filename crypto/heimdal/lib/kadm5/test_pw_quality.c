
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

#include "kadm5_locl.h"
#include <getarg.h>

RCSID("$Id$");

static int version_flag;
static int help_flag;
static char *principal;
static char *password;

static struct getargs args[] = {
    { "principal", 0, arg_string, &principal },
    { "password", 0, arg_string, &password },
    { "version", 0, arg_flag, &version_flag },
    { "help", 0, arg_flag, &help_flag }
};
int num_args = sizeof(args) / sizeof(args[0]);

int
main(int argc, char **argv)
{
    krb5_error_code ret;
    krb5_context context;
    krb5_principal p;
    const char *s;
    krb5_data pw_data;

    krb5_program_setup(&context, argc, argv, args, num_args, NULL);

    if(help_flag)
	krb5_std_usage(0, args, num_args);
    if(version_flag) {
	print_version(NULL);
	exit(0);
    }

    if (principal == NULL)
	krb5_errx(context, 1, "no principal given");
    if (password == NULL)
	krb5_errx(context, 1, "no password given");

    ret = krb5_parse_name(context, principal, &p);
    if (ret)
	krb5_errx(context, 1, "krb5_parse_name: %s", principal);

    pw_data.data = password;
    pw_data.length = strlen(password);

    kadm5_setup_passwd_quality_check (context, NULL, NULL);
    ret = kadm5_add_passwd_quality_verifier(context, NULL);
    if (ret)
	krb5_errx(context, 1, "kadm5_add_passwd_quality_verifier");

    s = kadm5_check_password_quality (context, p, &pw_data);
    if (s)
	krb5_errx(context, 1, "kadm5_check_password_quality:\n%s", s);

    krb5_free_principal(context, p);
    krb5_free_context(context);

    return 0;
}