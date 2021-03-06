
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

#include "kuser_locl.h"

static int help_flag = 0;
static int version_flag = 0;

static struct getargs args[] = {
    { "version", 	0,   arg_flag, &version_flag },
    { "help",		0,   arg_flag, &help_flag }
};

static void
usage (int ret)
{
    arg_printusage (args,
		    sizeof(args)/sizeof(*args),
		    NULL,
		    "[principal]");
    exit (ret);
}

int
main(int argc, char **argv)
{
    krb5_context context;
    krb5_error_code ret;
    krb5_creds cred;
    krb5_preauthtype pre_auth_types[] = {KRB5_PADATA_ENC_TIMESTAMP};
    krb5_get_init_creds_opt *get_options;
    krb5_verify_init_creds_opt verify_options;
    krb5_principal principal = NULL;
    int optidx = 0;

    setprogname (argv[0]);

    if(getarg(args, sizeof(args) / sizeof(args[0]), argc, argv, &optidx))
	usage(1);

    if (help_flag)
	usage (0);

    if(version_flag) {
	print_version(NULL);
	exit(0);
    }

    argc -= optidx;
    argv += optidx;

    ret = krb5_init_context(&context);
    if (ret)
	errx (1, "krb5_init_context failed: %d", ret);

    ret = krb5_get_init_creds_opt_alloc (context, &get_options);
    if (ret)
	krb5_err(context, 1, ret, "krb5_get_init_creds_opt_alloc");

    krb5_get_init_creds_opt_set_preauth_list (get_options,
					      pre_auth_types,
					      1);

    krb5_verify_init_creds_opt_init (&verify_options);

    if (argc) {
	ret = krb5_parse_name(context, argv[0], &principal);
	if (ret)
	    krb5_err(context, 1, ret, "krb5_parse_name: %s", argv[0]);
    } else {
	ret = krb5_get_default_principal(context, &principal);
	if (ret)
	    krb5_err(context, 1, ret, "krb5_get_default_principal");

    }

    ret = krb5_get_init_creds_password (context,
					&cred,
					principal,
					NULL,
					krb5_prompter_posix,
					NULL,
					0,
					NULL,
					get_options);
    if (ret)
	krb5_err(context, 1, ret,  "krb5_get_init_creds");

    ret = krb5_verify_init_creds (context,
				  &cred,
				  NULL,
				  NULL,
				  NULL,
				  &verify_options);
    if (ret)
	krb5_err(context, 1, ret, "krb5_verify_init_creds");
    krb5_free_cred_contents (context, &cred);
    krb5_free_context (context);
    return 0;
}