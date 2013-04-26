
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

static void
test_alname(krb5_context context, krb5_const_realm realm,
	    const char *user, const char *inst,
	    const char *localuser, int ok)
{
    krb5_principal p;
    char localname[1024];
    krb5_error_code ret;
    char *princ;

    ret = krb5_make_principal(context, &p, realm, user, inst, NULL);
    if (ret)
	krb5_err(context, 1, ret, "krb5_build_principal");

    ret = krb5_unparse_name(context, p, &princ);
    if (ret)
	krb5_err(context, 1, ret, "krb5_unparse_name");

    ret = krb5_aname_to_localname(context, p, sizeof(localname), localname);
    krb5_free_principal(context, p);
    free(princ);
    if (ret) {
	if (!ok)
	    return;
	krb5_err(context, 1, ret, "krb5_aname_to_localname: %s -> %s",
		 princ, localuser);
    }

    if (strcmp(localname, localuser) != 0) {
	if (ok)
	    errx(1, "compared failed %s != %s (should have succeded)",
		 localname, localuser);
    } else {
	if (!ok)
	    errx(1, "compared failed %s == %s (should have failed)",
		 localname, localuser);
    }

}

static int version_flag = 0;
static int help_flag	= 0;

static struct getargs args[] = {
    {"version",	0,	arg_flag,	&version_flag,
     "print version", NULL },
    {"help",	0,	arg_flag,	&help_flag,
     NULL, NULL }
};

static void
usage (int ret)
{
    arg_printusage (args,
		    sizeof(args)/sizeof(*args),
		    NULL,
		    "");
    exit (ret);
}

int
main(int argc, char **argv)
{
    krb5_context context;
    krb5_error_code ret;
    krb5_realm realm;
    int optidx = 0;
    char *user;

    setprogname(argv[0]);

    if(getarg(args, sizeof(args) / sizeof(args[0]), argc, argv, &optidx))
	usage(1);

    if (help_flag)
	usage (0);

    if(version_flag){
	print_version(NULL);
	exit(0);
    }

    argc -= optidx;
    argv += optidx;

    if (argc != 1)
	errx(1, "first argument should be a local user that in root .k5login");

    user = argv[0];

    ret = krb5_init_context(&context);
    if (ret)
	errx (1, "krb5_init_context failed: %d", ret);

    ret = krb5_get_default_realm(context, &realm);
    if (ret)
	krb5_err(context, 1, ret, "krb5_get_default_realm");

    test_alname(context, realm, user, NULL, user, 1);
    test_alname(context, realm, user, "root", "root", 1);

    test_alname(context, "FOO.BAR.BAZ.KAKA", user, NULL, user, 0);
    test_alname(context, "FOO.BAR.BAZ.KAKA", user, "root", "root", 0);

    test_alname(context, realm, user, NULL,
		"not-same-as-user", 0);
    test_alname(context, realm, user, "root",
		"not-same-as-user", 0);

    test_alname(context, "FOO.BAR.BAZ.KAKA", user, NULL,
		"not-same-as-user", 0);
    test_alname(context, "FOO.BAR.BAZ.KAKA", user, "root",
		"not-same-as-user", 0);

    krb5_free_context(context);

    return 0;
}