
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

static HEIMDAL_MUTEX context_mutex = HEIMDAL_MUTEX_INITIALIZER;
static int created_key;
static HEIMDAL_thread_key context_key;

static void
destroy_context(void *ptr)
{
    krb5_context context = ptr;

    if (context == NULL)
	return;
    krb5_free_context(context);
}

krb5_error_code
_gsskrb5_init (krb5_context *context)
{
    krb5_error_code ret = 0;

    HEIMDAL_MUTEX_lock(&context_mutex);

    if (!created_key) {
	HEIMDAL_key_create(&context_key, destroy_context, ret);
	if (ret) {
	    HEIMDAL_MUTEX_unlock(&context_mutex);
	    return ret;
	}
	created_key = 1;
    }
    HEIMDAL_MUTEX_unlock(&context_mutex);

    *context = HEIMDAL_getspecific(context_key);
    if (*context == NULL) {

	ret = krb5_init_context(context);
	if (ret == 0) {
	    HEIMDAL_setspecific(context_key, *context, ret);
	    if (ret) {
		krb5_free_context(*context);
		*context = NULL;
	    }
	}
    }

    return ret;
}