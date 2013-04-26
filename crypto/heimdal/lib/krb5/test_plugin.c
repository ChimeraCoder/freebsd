
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

#include <krb5_locl.h>
#include "locate_plugin.h"

static krb5_error_code
resolve_init(krb5_context context, void **ctx)
{
    *ctx = NULL;
    return 0;
}

static void
resolve_fini(void *ctx)
{
}

static krb5_error_code
resolve_lookup(void *ctx,
	       enum locate_service_type service,
	       const char *realm,
	       int domain,
	       int type,
	       int (*add)(void *,int,struct sockaddr *),
	       void *addctx)
{
    struct sockaddr_in s;

    memset(&s, 0, sizeof(s));

#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
    s.sin_len = sizeof(s);
#endif
    s.sin_family = AF_INET;
    s.sin_port = htons(88);
    s.sin_addr.s_addr = htonl(0x7f000002);

    if (strcmp(realm, "NOTHERE.H5L.SE") == 0)
	(*add)(addctx, type, (struct sockaddr *)&s);

    return 0;
}


krb5plugin_service_locate_ftable resolve = {
    0,
    resolve_init,
    resolve_fini,
    resolve_lookup
};


int
main(int argc, char **argv)
{
    krb5_error_code ret;
    krb5_context context;
    krb5_krbhst_handle handle;
    char host[MAXHOSTNAMELEN];
    int found = 0;

    setprogname(argv[0]);

    ret = krb5_init_context(&context);
    if (ret)
	errx(1, "krb5_init_contex");

    ret = krb5_plugin_register(context, PLUGIN_TYPE_DATA,
			       KRB5_PLUGIN_LOCATE, &resolve);
    if (ret)
	krb5_err(context, 1, ret, "krb5_plugin_register");


    ret = krb5_krbhst_init_flags(context,
				 "NOTHERE.H5L.SE",
				 KRB5_KRBHST_KDC,
				 0,
				 &handle);
    if (ret)
	krb5_err(context, 1, ret, "krb5_krbhst_init_flags");


    while(krb5_krbhst_next_as_string(context, handle, host, sizeof(host)) == 0){
	found++;
 	if (strcmp(host, "127.0.0.2") != 0)
	    krb5_errx(context, 1, "wrong address: %s", host);
    }
    if (!found)
	krb5_errx(context, 1, "failed to find host");

    krb5_krbhst_free(context, handle);

    krb5_free_context(context);
    return 0;
}