
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <roken.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <gssapi.h>
#include <gssapi_krb5.h>
#include <gssapi_spnego.h>
#include <err.h>
#include <getarg.h>

static void
gss_print_errors (int min_stat)
{
    OM_uint32 new_stat;
    OM_uint32 msg_ctx = 0;
    gss_buffer_desc status_string;
    OM_uint32 ret;

    do {
	ret = gss_display_status (&new_stat,
				  min_stat,
				  GSS_C_MECH_CODE,
				  GSS_C_NO_OID,
				  &msg_ctx,
				  &status_string);
	if (!GSS_ERROR(ret)) {
	    fprintf (stderr, "%.*s\n", (int)status_string.length,
					(char *)status_string.value);
	    gss_release_buffer (&new_stat, &status_string);
	}
    } while (!GSS_ERROR(ret) && msg_ctx != 0);
}

static void
gss_err(int exitval, int status, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vwarnx (fmt, args);
    gss_print_errors (status);
    va_end(args);
    exit (exitval);
}

static int version_flag = 0;
static int help_flag	= 0;

static struct getargs args[] = {
    {"version",	0,	arg_flag,	&version_flag, "print version", NULL },
    {"help",	0,	arg_flag,	&help_flag,  NULL, NULL }
};

static void
usage (int ret)
{
    arg_printusage (args, sizeof(args)/sizeof(*args),
		    NULL, "service@host");
    exit (ret);
}


int
main(int argc, char **argv)
{
    gss_buffer_desc name_buffer;
    OM_uint32 maj_stat, min_stat;
    gss_name_t name, MNname, MNname2;
    int optidx = 0;
    char *str;
    int len, equal;

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

    gsskrb5_set_default_realm("MIT.EDU");

    /*
     * test import/export
     */

    str = NULL;
    len = asprintf(&str, "ftp@freeze-arrow.mit.edu");
    if (len < 0 || str == NULL)
	errx(1, "asprintf");

    name_buffer.value = str;
    name_buffer.length = len;

    maj_stat = gss_import_name(&min_stat, &name_buffer,
			       GSS_C_NT_HOSTBASED_SERVICE,
			       &name);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "import name error");
    free(str);

    maj_stat = gss_canonicalize_name (&min_stat,
				      name,
				      GSS_KRB5_MECHANISM,
				      &MNname);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "canonicalize name error");

    maj_stat = gss_export_name(&min_stat,
			       MNname,
			       &name_buffer);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "export name error (KRB5)");

    /*
     * Import the exported name and compare
     */

    maj_stat = gss_import_name(&min_stat, &name_buffer,
			       GSS_C_NT_EXPORT_NAME,
			       &MNname2);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "import name error (exported KRB5 name)");


    maj_stat = gss_compare_name(&min_stat, MNname, MNname2, &equal);
    if (maj_stat != GSS_S_COMPLETE)
	errx(1, "gss_compare_name");
    if (!equal)
	errx(1, "names not equal");

    gss_release_name(&min_stat, &MNname2);
    gss_release_buffer(&min_stat, &name_buffer);
    gss_release_name(&min_stat, &MNname);
    gss_release_name(&min_stat, &name);

    /*
     * Import oid less name and compare to mech name.
     * Dovecot SASL lib does this.
     */

    str = NULL;
    len = asprintf(&str, "lha");
    if (len < 0 || str == NULL)
	errx(1, "asprintf");

    name_buffer.value = str;
    name_buffer.length = len;

    maj_stat = gss_import_name(&min_stat, &name_buffer,
			       GSS_C_NO_OID,
			       &name);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "import (no oid) name error");

    maj_stat = gss_import_name(&min_stat, &name_buffer,
			       GSS_KRB5_NT_USER_NAME,
			       &MNname);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "import (krb5 mn) name error");

    free(str);

    maj_stat = gss_compare_name(&min_stat, name, MNname, &equal);
    if (maj_stat != GSS_S_COMPLETE)
	errx(1, "gss_compare_name");
    if (!equal)
	errx(1, "names not equal");

    gss_release_name(&min_stat, &MNname);
    gss_release_name(&min_stat, &name);

#if 0
    maj_stat = gss_canonicalize_name (&min_stat,
				      name,
				      GSS_SPNEGO_MECHANISM,
				      &MNname);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "canonicalize name error");


    maj_stat = gss_export_name(&maj_stat,
			       MNname,
			       &name_buffer);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "export name error (SPNEGO)");

    gss_release_name(&min_stat, &MNname);
    gss_release_buffer(&min_stat, &name_buffer);
#endif

    return 0;
}