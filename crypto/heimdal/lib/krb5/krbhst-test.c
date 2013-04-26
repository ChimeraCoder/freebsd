
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
		    "[realms ...]");
    exit (ret);
}

int
main(int argc, char **argv)
{
    int i, j;
    krb5_context context;
    int types[] = {KRB5_KRBHST_KDC, KRB5_KRBHST_ADMIN, KRB5_KRBHST_CHANGEPW,
		   KRB5_KRBHST_KRB524};
    const char *type_str[] = {"kdc", "admin", "changepw", "krb524"};
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

    argc -= optidx;
    argv += optidx;

    krb5_init_context (&context);
    for(i = 0; i < argc; i++) {
	krb5_krbhst_handle handle;
	char host[MAXHOSTNAMELEN];

	for (j = 0; j < sizeof(types)/sizeof(*types); ++j) {
	    printf ("%s for %s:\n", type_str[j], argv[i]);

	    krb5_krbhst_init(context, argv[i], types[j], &handle);
	    while(krb5_krbhst_next_as_string(context, handle,
					     host, sizeof(host)) == 0)
		printf("\thost: %s\n", host);
	    krb5_krbhst_reset(context, handle);
	    printf ("\n");
	}
    }
    return 0;
}