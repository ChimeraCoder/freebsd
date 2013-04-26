
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

#include "test_common.h"

static void
print_time(OM_uint32 time_rec)
{
    if (time_rec == GSS_C_INDEFINITE) {
	printf("cred never expire\n");
    } else {
	time_t t = time_rec + time(NULL);
	printf("expiration time: %s", ctime(&t));
    }
}

#if 0

static void
test_add(gss_cred_id_t cred_handle)
{
    OM_uint32 major_status, minor_status;
    gss_cred_id_t copy_cred;
    OM_uint32 time_rec;

    major_status = gss_add_cred (&minor_status,
				 cred_handle,
				 GSS_C_NO_NAME,
				 GSS_KRB5_MECHANISM,
				 GSS_C_INITIATE,
				 0,
				 0,
				 &copy_cred,
				 NULL,
				 &time_rec,
				 NULL);

    if (GSS_ERROR(major_status))
	errx(1, "add_cred failed");

    print_time(time_rec);

    major_status = gss_release_cred(&minor_status,
				    &copy_cred);
    if (GSS_ERROR(major_status))
	errx(1, "release_cred failed");
}

static void
copy_cred(void)
{
    OM_uint32 major_status, minor_status;
    gss_cred_id_t cred_handle;
    OM_uint32 time_rec;

    major_status = gss_acquire_cred(&minor_status,
				    GSS_C_NO_NAME,
				    0,
				    NULL,
				    GSS_C_INITIATE,
				    &cred_handle,
				    NULL,
				    &time_rec);
    if (GSS_ERROR(major_status))
	errx(1, "acquire_cred failed");

    print_time(time_rec);

    test_add(cred_handle);
    test_add(cred_handle);
    test_add(cred_handle);

    major_status = gss_release_cred(&minor_status,
				    &cred_handle);
    if (GSS_ERROR(major_status))
	errx(1, "release_cred failed");
}
#endif

static gss_cred_id_t
acquire_cred_service(const char *service,
		     gss_OID nametype,
		     gss_OID_set oidset,
		     int flags)
{
    OM_uint32 major_status, minor_status;
    gss_cred_id_t cred_handle;
    OM_uint32 time_rec;
    gss_buffer_desc name_buffer;
    gss_name_t name = GSS_C_NO_NAME;

    if (service) {
	name_buffer.value = rk_UNCONST(service);
	name_buffer.length = strlen(service);

	major_status = gss_import_name(&minor_status,
				       &name_buffer,
				       nametype,
				       &name);
	if (GSS_ERROR(major_status))
	    errx(1, "import_name failed");
    }

    major_status = gss_acquire_cred(&minor_status,
				    name,
				    0,
				    oidset,
				    flags,
				    &cred_handle,
				    NULL,
				    &time_rec);
    if (GSS_ERROR(major_status)) {
	warnx("acquire_cred failed: %s",
	     gssapi_err(major_status, minor_status, GSS_C_NO_OID));
    } else {
	print_time(time_rec);
	gss_release_cred(&minor_status, &cred_handle);
    }

    if (name != GSS_C_NO_NAME)
	gss_release_name(&minor_status, &name);

    if (GSS_ERROR(major_status))
	exit(1);

    return cred_handle;
}

static int version_flag = 0;
static int help_flag	= 0;
static int kerberos_flag = 0;
static int enctype = 0;
static char *acquire_name;
static char *acquire_type;
static char *target_name;
static char *name_type;
static char *ccache;
static int num_loops = 1;

static struct getargs args[] = {
    {"acquire-name", 0,	arg_string,	&acquire_name, "name", NULL },
    {"acquire-type", 0,	arg_string,	&acquire_type, "type", NULL },
    {"enctype", 0,	arg_integer,	&enctype, "enctype-num", NULL },
    {"loops", 0,	arg_integer,	&num_loops, "enctype-num", NULL },
    {"kerberos", 0,	arg_flag,	&kerberos_flag, "enctype-num", NULL },
    {"target-name", 0,	arg_string,	&target_name, "name", NULL },
    {"ccache", 0,	arg_string,	&ccache, "name", NULL },
    {"name-type", 0,	arg_string,	&name_type, "type", NULL },
    {"version",	0,	arg_flag,	&version_flag, "print version", NULL },
    {"help",	0,	arg_flag,	&help_flag,  NULL, NULL }
};

static void
usage (int ret)
{
    arg_printusage (args, sizeof(args)/sizeof(*args), NULL, "");
    exit (ret);
}

int
main(int argc, char **argv)
{
    gss_OID_set oidset = GSS_C_NULL_OID_SET;
    gss_OID mechoid = GSS_C_NO_OID;
    OM_uint32 maj_stat, min_stat;
    gss_cred_id_t cred;
    gss_name_t target = GSS_C_NO_NAME;
    int i, optidx = 0;
    OM_uint32 flag;
    gss_OID type;

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

    if (argc != 0)
	usage(1);

    if (acquire_type) {
	if (strcasecmp(acquire_type, "both") == 0)
	    flag = GSS_C_BOTH;
	else if (strcasecmp(acquire_type, "accept") == 0)
	    flag = GSS_C_ACCEPT;
	else if (strcasecmp(acquire_type, "initiate") == 0)
	    flag = GSS_C_INITIATE;
	else
	    errx(1, "unknown type %s", acquire_type);
    } else
	flag = GSS_C_ACCEPT;

    if (name_type) {
	if (strcasecmp("hostbased-service", name_type) == 0)
	    type = GSS_C_NT_HOSTBASED_SERVICE;
	else if (strcasecmp("user-name", name_type) == 0)
	    type = GSS_C_NT_USER_NAME;
	else
	    errx(1, "unknown name type %s", name_type);
    } else
	type = GSS_C_NT_HOSTBASED_SERVICE;

    if (ccache) {
	maj_stat = gss_krb5_ccache_name(&min_stat, ccache, NULL);
	if (GSS_ERROR(maj_stat))
	    errx(1, "gss_krb5_ccache_name %s",
		 gssapi_err(maj_stat, min_stat, GSS_C_NO_OID));
    }

    if (kerberos_flag) {
	mechoid = GSS_KRB5_MECHANISM;

	maj_stat = gss_create_empty_oid_set(&min_stat, &oidset);
	if (maj_stat != GSS_S_COMPLETE)
	    errx(1, "gss_create_empty_oid_set: %s",
		 gssapi_err(maj_stat, min_stat, GSS_C_NO_OID));

	maj_stat = gss_add_oid_set_member(&min_stat, GSS_KRB5_MECHANISM, &oidset);
	if (maj_stat != GSS_S_COMPLETE)
	    errx(1, "gss_add_oid_set_member: %s",
		 gssapi_err(maj_stat, min_stat, GSS_C_NO_OID));
    }

    if (target_name) {
	gss_buffer_desc name;

	name.value = target_name;
	name.length = strlen(target_name);
	maj_stat = gss_import_name(&min_stat, &name,
				   GSS_C_NT_HOSTBASED_SERVICE, &target);
	if (maj_stat != GSS_S_COMPLETE)
	    errx(1, "gss_import_name: %s",
		 gssapi_err(maj_stat, min_stat, GSS_C_NO_OID));
    }

    for (i = 0; i < num_loops; i++) {

	cred = acquire_cred_service(acquire_name, type, oidset, flag);

	if (enctype) {
	    int32_t enctypelist = enctype;

	    maj_stat = gss_krb5_set_allowable_enctypes(&min_stat, cred,
						       1, &enctypelist);
	    if (maj_stat)
		errx(1, "gss_krb5_set_allowable_enctypes: %s",
		     gssapi_err(maj_stat, min_stat, GSS_C_NO_OID));
	}

	if (target) {
	    gss_ctx_id_t context = GSS_C_NO_CONTEXT;
	    gss_buffer_desc out;

	    out.length = 0;
	    out.value = NULL;

	    maj_stat = gss_init_sec_context(&min_stat,
					    cred, &context,
					    target, mechoid,
					    GSS_C_MUTUAL_FLAG, 0, NULL,
					    GSS_C_NO_BUFFER, NULL,
					    &out, NULL, NULL);
	    if (maj_stat != GSS_S_COMPLETE && maj_stat != GSS_S_CONTINUE_NEEDED)
		errx(1, "init_sec_context failed: %s",
		     gssapi_err(maj_stat, min_stat, GSS_C_NO_OID));

	    gss_release_buffer(&min_stat, &out);
	    gss_delete_sec_context(&min_stat, &context, NULL);
	}
	gss_release_cred(&min_stat, &cred);
    }


    return 0;
}