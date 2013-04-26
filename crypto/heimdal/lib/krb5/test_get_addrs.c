
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
#include <getarg.h>

/* print all addresses that we find */

static void
print_addresses (krb5_context context, const krb5_addresses *addrs)
{
    int i;
    char buf[256];
    size_t len;

    for (i = 0; i < addrs->len; ++i) {
	krb5_print_address (&addrs->val[i], buf, sizeof(buf), &len);
	printf ("%s\n", buf);
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
    krb5_addresses addrs;
    int optidx = 0;

    setprogname (argv[0]);

    if(getarg(args, sizeof(args) / sizeof(args[0]), argc, argv, &optidx))
	usage(1);

    if (help_flag)
	usage (0);

    if(version_flag){
	print_version(NULL);
	exit(0);
    }

    ret = krb5_init_context(&context);
    if (ret)
	errx (1, "krb5_init_context failed: %d", ret);

    ret = krb5_get_all_client_addrs (context, &addrs);
    if (ret)
	krb5_err (context, 1, ret, "krb5_get_all_client_addrs");
    printf ("client addresses\n");
    print_addresses (context, &addrs);
    krb5_free_addresses (context, &addrs);

    ret = krb5_get_all_server_addrs (context, &addrs);
    if (ret)
	krb5_err (context, 1, ret, "krb5_get_all_server_addrs");
    printf ("server addresses\n");
    print_addresses (context, &addrs);
    krb5_free_addresses (context, &addrs);
    return 0;
}