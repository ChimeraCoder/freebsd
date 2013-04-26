
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

/* These are stub functions for the standalone RFC3961 crypto library */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_init_context(krb5_context *context)
{
    krb5_context p;

    *context = NULL;

    /* should have a run_once */
    bindtextdomain(HEIMDAL_TEXTDOMAIN, HEIMDAL_LOCALEDIR);

    p = calloc(1, sizeof(*p));
    if(!p)
        return ENOMEM;

    p->mutex = malloc(sizeof(HEIMDAL_MUTEX));
    if (p->mutex == NULL) {
        free(p);
        return ENOMEM;
    }
    HEIMDAL_MUTEX_init(p->mutex);

    *context = p;
    return 0;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_context(krb5_context context)
{
    krb5_clear_error_message(context);

    HEIMDAL_MUTEX_destroy(context->mutex);
    free(context->mutex);
    if (context->flags & KRB5_CTX_F_SOCKETS_INITIALIZED) {
        rk_SOCK_EXIT();
    }

    memset(context, 0, sizeof(*context));
    free(context);
}

krb5_boolean
_krb5_homedir_access(krb5_context context) {
    return 0;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_log(krb5_context context,
         krb5_log_facility *fac,
         int level,
         const char *fmt,
         ...)
{
    return 0;
}

/* This function is currently just used to get the location of the EGD
 * socket. If we're not using an EGD, then we can just return NULL */

KRB5_LIB_FUNCTION const char* KRB5_LIB_CALL
krb5_config_get_string (krb5_context context,
                        const krb5_config_section *c,
                        ...)
{
    return NULL;
}