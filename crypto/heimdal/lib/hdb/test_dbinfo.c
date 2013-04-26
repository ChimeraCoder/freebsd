
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

#include "hdb_locl.h"
#include <getarg.h>

static int help_flag;
static int version_flag;

struct getargs args[] = {
    { "help",		'h',	arg_flag,   &help_flag },
    { "version",	0,	arg_flag,   &version_flag }
};

static int num_args = sizeof(args) / sizeof(args[0]);

int
main(int argc, char **argv)
{
    struct hdb_dbinfo *info, *d;
    krb5_context context;
    int ret, o = 0;

    setprogname(argv[0]);

    if(getarg(args, num_args, argc, argv, &o))
	krb5_std_usage(1, args, num_args);

    if(help_flag)
	krb5_std_usage(0, args, num_args);

    if(version_flag){
	print_version(NULL);
	exit(0);
    }

    ret = krb5_init_context(&context);
    if (ret)
	errx (1, "krb5_init_context failed: %d", ret);

    ret = hdb_get_dbinfo(context, &info);
    if (ret)
	krb5_err(context, 1, ret, "hdb_get_dbinfo");

    d = NULL;
    while ((d = hdb_dbinfo_get_next(info, d)) != NULL) {
	const char *s;
	s = hdb_dbinfo_get_label(context, d);
	printf("label: %s\n", s ? s : "no label");
	s = hdb_dbinfo_get_realm(context, d);
	printf("\trealm: %s\n", s ? s : "no realm");
	s = hdb_dbinfo_get_dbname(context, d);
	printf("\tdbname: %s\n", s ? s : "no dbname");
	s = hdb_dbinfo_get_mkey_file(context, d);
	printf("\tmkey_file: %s\n", s ? s : "no mkey file");
	s = hdb_dbinfo_get_acl_file(context, d);
	printf("\tacl_file: %s\n", s ? s : "no acl file");
    }

    hdb_free_dbinfo(context, &info);

    krb5_free_context(context);

    return 0;
}