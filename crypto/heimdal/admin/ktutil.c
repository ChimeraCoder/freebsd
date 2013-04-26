
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

#include "ktutil_locl.h"
#include <err.h>

RCSID("$Id$");

static int help_flag;
static int version_flag;
int verbose_flag;
char *keytab_string;
static char keytab_buf[256];

static struct getargs args[] = {
    {
	"version",
	0,
	arg_flag,
	&version_flag,
	NULL,
	NULL
    },
    {
	"help",
	'h',
	arg_flag,
	&help_flag,
	NULL,
	NULL
    },
    {
	"keytab",
	'k',
	arg_string,
	&keytab_string,
	"keytab",
	"keytab to operate on"
    },
    {
	"verbose",
	'v',
	arg_flag,
	&verbose_flag,
	"verbose",
	"run verbosely"
    }
};

static int num_args = sizeof(args) / sizeof(args[0]);

krb5_context context;

krb5_keytab
ktutil_open_keytab(void)
{
    krb5_error_code ret;
    krb5_keytab keytab;
    if (keytab_string == NULL) {
	ret = krb5_kt_default_name (context, keytab_buf, sizeof(keytab_buf));
	if (ret) {
	    krb5_warn(context, ret, "krb5_kt_default_name");
	    return NULL;
	}
	keytab_string = keytab_buf;
    }
    ret = krb5_kt_resolve(context, keytab_string, &keytab);
    if (ret) {
	krb5_warn(context, ret, "resolving keytab %s", keytab_string);
	return NULL;
    }
    if (verbose_flag)
	fprintf (stderr, "Using keytab %s\n", keytab_string);

    return keytab;
}

int
help(void *opt, int argc, char **argv)
{
    if(argc == 0) {
	sl_help(commands, 1, argv - 1 /* XXX */);
    } else {
	SL_cmd *c = sl_match (commands, argv[0], 0);
 	if(c == NULL) {
	    fprintf (stderr, "No such command: %s. "
		     "Try \"help\" for a list of commands\n",
		     argv[0]);
	} else {
	    if(c->func) {
		char shelp[] = "--help";
		char *fake[3];
		fake[0] = argv[0];
		fake[1] = shelp;
		fake[2] = NULL;
		(*c->func)(2, fake);
		fprintf(stderr, "\n");
	    }
	    if(c->help && *c->help)
		fprintf (stderr, "%s\n", c->help);
	    if((++c)->name && c->func == NULL) {
		int f = 0;
		fprintf (stderr, "Synonyms:");
		while (c->name && c->func == NULL) {
		    fprintf (stderr, "%s%s", f ? ", " : " ", (c++)->name);
		    f = 1;
		}
		fprintf (stderr, "\n");
	    }
	}
    }
    return 0;
}

static void
usage(int status)
{
    arg_printusage(args, num_args, NULL, "command");
    exit(status);
}

int
main(int argc, char **argv)
{
    int optidx = 0;
    krb5_error_code ret;
    setprogname(argv[0]);
    ret = krb5_init_context(&context);
    if (ret)
	errx (1, "krb5_init_context failed: %d", ret);
    if(getarg(args, num_args, argc, argv, &optidx))
	usage(1);
    if(help_flag)
	usage(0);
    if(version_flag) {
	print_version(NULL);
	exit(0);
    }
    argc -= optidx;
    argv += optidx;
    if(argc == 0)
	usage(1);
    ret = sl_command(commands, argc, argv);
    if(ret == -1)
	krb5_warnx (context, "unrecognized command: %s", argv[0]);
    return ret;
}