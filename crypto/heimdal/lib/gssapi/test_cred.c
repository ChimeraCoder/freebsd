
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

static void
acquire_release_loop(gss_name_t name, int counter, gss_cred_usage_t usage)
{
    OM_uint32 maj_stat, min_stat;
    gss_cred_id_t cred;
    int i;

    for (i = 0; i < counter; i++) {
	maj_stat = gss_acquire_cred(&min_stat, name,
				    GSS_C_INDEFINITE,
				    GSS_C_NO_OID_SET,
				    usage,
				    &cred,
				    NULL,
				    NULL);
	if (maj_stat != GSS_S_COMPLETE)
	    gss_err(1, min_stat, "aquire %d %d != GSS_S_COMPLETE",
		    i, (int)maj_stat);

	maj_stat = gss_release_cred(&min_stat, &cred);
	if (maj_stat != GSS_S_COMPLETE)
	    gss_err(1, min_stat, "release %d %d != GSS_S_COMPLETE",
		    i, (int)maj_stat);
    }
}


static void
acquire_add_release_add(gss_name_t name, gss_cred_usage_t usage)
{
    OM_uint32 maj_stat, min_stat;
    gss_cred_id_t cred, cred2, cred3;

    maj_stat = gss_acquire_cred(&min_stat, name,
				GSS_C_INDEFINITE,
				GSS_C_NO_OID_SET,
				usage,
				&cred,
				NULL,
				NULL);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "aquire %d != GSS_S_COMPLETE", (int)maj_stat);

    maj_stat = gss_add_cred(&min_stat,
			    cred,
			    GSS_C_NO_NAME,
			    GSS_KRB5_MECHANISM,
			    usage,
			    GSS_C_INDEFINITE,
			    GSS_C_INDEFINITE,
			    &cred2,
			    NULL,
			    NULL,
			    NULL);

    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "add_cred %d != GSS_S_COMPLETE", (int)maj_stat);

    maj_stat = gss_release_cred(&min_stat, &cred);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "release %d != GSS_S_COMPLETE", (int)maj_stat);

    maj_stat = gss_add_cred(&min_stat,
			    cred2,
			    GSS_C_NO_NAME,
			    GSS_KRB5_MECHANISM,
			    GSS_C_BOTH,
			    GSS_C_INDEFINITE,
			    GSS_C_INDEFINITE,
			    &cred3,
			    NULL,
			    NULL,
			    NULL);

    maj_stat = gss_release_cred(&min_stat, &cred2);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "release 2 %d != GSS_S_COMPLETE", (int)maj_stat);

    maj_stat = gss_release_cred(&min_stat, &cred3);
    if (maj_stat != GSS_S_COMPLETE)
	gss_err(1, min_stat, "release 2 %d != GSS_S_COMPLETE", (int)maj_stat);
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
    struct gss_buffer_desc_struct name_buffer;
    OM_uint32 maj_stat, min_stat;
    gss_name_t name;
    int optidx = 0;

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

    if (argc < 1)
	errx(1, "argc < 1");

    name_buffer.value = argv[0];
    name_buffer.length = strlen(argv[0]);

    maj_stat = gss_import_name(&min_stat, &name_buffer,
			       GSS_C_NT_HOSTBASED_SERVICE,
			       &name);
    if (maj_stat != GSS_S_COMPLETE)
	errx(1, "import name error");

    acquire_release_loop(name, 100, GSS_C_ACCEPT);
    acquire_release_loop(name, 100, GSS_C_INITIATE);
    acquire_release_loop(name, 100, GSS_C_BOTH);

    acquire_add_release_add(name, GSS_C_ACCEPT);
    acquire_add_release_add(name, GSS_C_INITIATE);
    acquire_add_release_add(name, GSS_C_BOTH);

    gss_release_name(&min_stat, &name);

    return 0;
}