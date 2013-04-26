
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

#include "gsskrb5_locl.h"

char *last_out_name;

OM_uint32
_gsskrb5_krb5_ccache_name(OM_uint32 *minor_status,
			  const char *name,
			  const char **out_name)
{
    krb5_context context;
    krb5_error_code kret;

    *minor_status = 0;

    GSSAPI_KRB5_INIT(&context);

    if (out_name) {
	const char *n;

	if (last_out_name) {
	    free(last_out_name);
	    last_out_name = NULL;
	}

	n = krb5_cc_default_name(context);
	if (n == NULL) {
	    *minor_status = ENOMEM;
	    return GSS_S_FAILURE;
	}
	last_out_name = strdup(n);
	if (last_out_name == NULL) {
	    *minor_status = ENOMEM;
	    return GSS_S_FAILURE;
	}
	*out_name = last_out_name;
    }

    kret = krb5_cc_set_default_name(context, name);
    if (kret) {
	*minor_status = kret;
	return GSS_S_FAILURE;
    }
    return GSS_S_COMPLETE;
}