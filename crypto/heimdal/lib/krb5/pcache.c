
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

#include "krb5_locl.h"
#include "ccache_plugin.h"
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#include <assert.h>

krb5_error_code
_krb5_load_ccache_plugins(krb5_context context)
{
    struct krb5_plugin * plist = NULL;
    struct krb5_plugin *p;
    krb5_error_code code;

    code = _krb5_plugin_find(context, PLUGIN_TYPE_DATA, KRB5_PLUGIN_CCACHE,
                             &plist);
    if (code)
        return code;

    for (p = plist; p != NULL; p = _krb5_plugin_get_next(p)) {
        krb5_cc_ops * ccops;
        krb5_error_code c_load;

        ccops = _krb5_plugin_get_symbol(p);
        if (ccops != NULL && ccops->version == KRB5_CC_OPS_VERSION) {
            c_load = krb5_cc_register(context, ccops, TRUE);
            if (c_load != 0)
                code = c_load;
        }
    }

    _krb5_plugin_free(plist);

    return code;
}