
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

#include "kdc_locl.h"
#include <getarg.h>
#include <parse_bytes.h>

struct dbinfo {
    char *realm;
    char *dbname;
    char *mkey_file;
    struct dbinfo *next;
};

static char *config_file;	/* location of kdc config file */

static int require_preauth = -1; /* 1 == require preauth for all principals */
static char *max_request_str;	/* `max_request' as a string */

static int disable_des = -1;

static int builtin_hdb_flag;
static int help_flag;
static int version_flag;

static struct getarg_strings addresses_str;	/* addresses to listen on */

char *runas_string;
char *chroot_string;


static struct getargs args[] = {
    {
	"config-file",	'c',	arg_string,	&config_file,
	"location of config file",	"file"
    },
    {
	"require-preauth",	'p',	arg_negative_flag, &require_preauth,
	"don't require pa-data in as-reqs", NULL
    },
    {
	"max-request",	0,	arg_string, &max_request_str,
	"max size for a kdc-request", "size"
    },
    { "enable-http", 'H', arg_flag, &enable_http, "turn on HTTP support",
   	 NULL },
    {	"ports",	'P', 	arg_string, rk_UNCONST(&port_str),
	"ports to listen to", "portspec"
    },
#ifdef SUPPORT_DETACH
#if DETACH_IS_DEFAULT
    {
	"detach",       'D',      arg_negative_flag, &detach_from_console,
	"don't detach from console", NULL
    },
#else
    {
	"detach",       0 ,      arg_flag, &detach_from_console,
	"detach from console", NULL
    },
#endif
#endif
    {	"addresses",	0,	arg_strings, &addresses_str,
	"addresses to listen on", "list of addresses" },
    {	"disable-des",	0,	arg_flag, &disable_des,
	"disable DES", NULL },
    {	"builtin-hdb",	0,	arg_flag,   &builtin_hdb_flag,
	"list builtin hdb backends", NULL},
    {   "runas-user",	0,	arg_string, &runas_string,
	"run as this user when connected to network", NULL
    },
    {   "chroot",	0,	arg_string, &chroot_string,
	"chroot directory to run in", NULL
    },
    {	"help",		'h',	arg_flag,   &help_flag, NULL, NULL },
    {	"version",	'v',	arg_flag,   &version_flag, NULL, NULL }
};

static int num_args = sizeof(args) / sizeof(args[0]);

static void
usage(int ret)
{
    arg_printusage (args, num_args, NULL, "");
    exit (ret);
}

static void
add_one_address (krb5_context context, const char *str, int first)
{
    krb5_error_code ret;
    krb5_addresses tmp;

    ret = krb5_parse_address (context, str, &tmp);
    if (ret)
	krb5_err (context, 1, ret, "parse_address `%s'", str);
    if (first)
	krb5_copy_addresses(context, &tmp, &explicit_addresses);
    else
	krb5_append_addresses(context, &explicit_addresses, &tmp);
    krb5_free_addresses (context, &tmp);
}

krb5_kdc_configuration *
configure(krb5_context context, int argc, char **argv)
{
    krb5_kdc_configuration *config;
    krb5_error_code ret;
    int optidx = 0;
    const char *p;

    while(getarg(args, num_args, argc, argv, &optidx))
	warnx("error at argument `%s'", argv[optidx]);

    if(help_flag)
	usage (0);

    if (version_flag) {
	print_version(NULL);
	exit(0);
    }

    if (builtin_hdb_flag) {
	char *list;
	ret = hdb_list_builtin(context, &list);
	if (ret)
	    krb5_err(context, 1, ret, "listing builtin hdb backends");
	printf("builtin hdb backends: %s\n", list);
	free(list);
	exit(0);
    }

    argc -= optidx;
    argv += optidx;

    if (argc != 0)
	usage(1);

    {
	char **files;

	if (config_file == NULL) {
	    asprintf(&config_file, "%s/kdc.conf", hdb_db_dir(context));
	    if (config_file == NULL)
		errx(1, "out of memory");
	}

	ret = krb5_prepend_config_files_default(config_file, &files);
	if (ret)
	    krb5_err(context, 1, ret, "getting configuration files");

	ret = krb5_set_config_files(context, files);
	krb5_free_config_files(files);
	if(ret)
	    krb5_err(context, 1, ret, "reading configuration files");
    }

    ret = krb5_kdc_get_config(context, &config);
    if (ret)
	krb5_err(context, 1, ret, "krb5_kdc_default_config");

    kdc_openlog(context, "kdc", config);

    ret = krb5_kdc_set_dbinfo(context, config);
    if (ret)
	krb5_err(context, 1, ret, "krb5_kdc_set_dbinfo");

    if(max_request_str)
	max_request_tcp = max_request_udp = parse_bytes(max_request_str, NULL);

    if(max_request_tcp == 0){
	p = krb5_config_get_string (context,
				    NULL,
				    "kdc",
				    "max-request",
				    NULL);
	if(p)
	    max_request_tcp = max_request_udp = parse_bytes(p, NULL);
    }

    if(require_preauth != -1)
	config->require_preauth = require_preauth;

    if(port_str == NULL){
	p = krb5_config_get_string(context, NULL, "kdc", "ports", NULL);
	if (p != NULL)
	    port_str = strdup(p);
    }

    explicit_addresses.len = 0;

    if (addresses_str.num_strings) {
	int i;

	for (i = 0; i < addresses_str.num_strings; ++i)
	    add_one_address (context, addresses_str.strings[i], i == 0);
	free_getarg_strings (&addresses_str);
    } else {
	char **foo = krb5_config_get_strings (context, NULL,
					      "kdc", "addresses", NULL);

	if (foo != NULL) {
	    add_one_address (context, *foo++, TRUE);
	    while (*foo)
		add_one_address (context, *foo++, FALSE);
	}
    }

    if(enable_http == -1)
	enable_http = krb5_config_get_bool(context, NULL, "kdc",
					   "enable-http", NULL);

    if(request_log == NULL)
	request_log = krb5_config_get_string(context, NULL,
					     "kdc",
					     "kdc-request-log",
					     NULL);

    if (krb5_config_get_string(context, NULL, "kdc",
			       "enforce-transited-policy", NULL))
	krb5_errx(context, 1, "enforce-transited-policy deprecated, "
		  "use [kdc]transited-policy instead");

#ifdef SUPPORT_DETACH
    if(detach_from_console == -1)
	detach_from_console = krb5_config_get_bool_default(context, NULL,
							   DETACH_IS_DEFAULT,
							   "kdc",
							   "detach", NULL);
#endif /* SUPPORT_DETACH */

    if(max_request_tcp == 0)
	max_request_tcp = 64 * 1024;
    if(max_request_udp == 0)
	max_request_udp = 64 * 1024;

    if (port_str == NULL)
	port_str = "+";

    if(disable_des == -1)
	disable_des = krb5_config_get_bool_default(context, NULL,
						   FALSE,
						   "kdc",
						   "disable-des", NULL);
    if(disable_des) {
	krb5_enctype_disable(context, ETYPE_DES_CBC_CRC);
	krb5_enctype_disable(context, ETYPE_DES_CBC_MD4);
	krb5_enctype_disable(context, ETYPE_DES_CBC_MD5);
	krb5_enctype_disable(context, ETYPE_DES_CBC_NONE);
	krb5_enctype_disable(context, ETYPE_DES_CFB64_NONE);
	krb5_enctype_disable(context, ETYPE_DES_PCBC_NONE);
    }

    krb5_kdc_windc_init(context);

    krb5_kdc_pkinit_config(context, config);

    return config;
}