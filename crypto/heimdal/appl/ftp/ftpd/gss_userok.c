
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

#include "ftpd_locl.h"
#include <gssapi/gssapi.h>

/* XXX sync with gssapi.c */
struct gssapi_data {
    gss_ctx_id_t context_hdl;
    gss_name_t client_name;
    gss_cred_id_t delegated_cred_handle;
    void *mech_data;
};

int gssapi_userok(void*, char*); /* to keep gcc happy */
int gssapi_session(void*, char*); /* to keep gcc happy */

int
gssapi_userok(void *app_data, char *username)
{
    struct gssapi_data *data = app_data;

    /* Yes, this logic really is inverted. */
    return !gss_userok(data->client_name, username);
}

int
gssapi_session(void *app_data, char *username)
{
    struct gssapi_data *data = app_data;
    OM_uint32 major, minor;
    int ret = 0;

    if (data->delegated_cred_handle != GSS_C_NO_CREDENTIAL) {
        major = gss_store_cred(&minor, data->delegated_cred_handle,
                               GSS_C_INITIATE, GSS_C_NO_OID,
                               1, 1, NULL, NULL);
        if (GSS_ERROR(major))
            ret = 1;
	afslog(NULL, 1);
    }

    gss_release_cred(&minor, &data->delegated_cred_handle);
    return ret;
}