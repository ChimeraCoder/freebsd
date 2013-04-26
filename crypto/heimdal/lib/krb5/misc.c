
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
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
_krb5_s4u2self_to_checksumdata(krb5_context context,
			       const PA_S4U2Self *self,
			       krb5_data *data)
{
    krb5_error_code ret;
    krb5_ssize_t ssize;
    krb5_storage *sp;
    size_t size;
    size_t i;

    sp = krb5_storage_emem();
    if (sp == NULL) {
	krb5_clear_error_message(context);
	return ENOMEM;
    }
    krb5_storage_set_flags(sp, KRB5_STORAGE_BYTEORDER_LE);
    ret = krb5_store_int32(sp, self->name.name_type);
    if (ret)
	goto out;
    for (i = 0; i < self->name.name_string.len; i++) {
	size = strlen(self->name.name_string.val[i]);
	ssize = krb5_storage_write(sp, self->name.name_string.val[i], size);
	if (ssize != (krb5_ssize_t)size) {
	    ret = ENOMEM;
	    goto out;
	}
    }
    size = strlen(self->realm);
    ssize = krb5_storage_write(sp, self->realm, size);
    if (ssize != (krb5_ssize_t)size) {
	ret = ENOMEM;
	goto out;
    }
    size = strlen(self->auth);
    ssize = krb5_storage_write(sp, self->auth, size);
    if (ssize != (krb5_ssize_t)size) {
	ret = ENOMEM;
	goto out;
    }

    ret = krb5_storage_to_data(sp, data);
    krb5_storage_free(sp);
    return ret;

out:
    krb5_clear_error_message(context);
    return ret;
}

krb5_error_code
krb5_enomem(krb5_context context)
{
    krb5_set_error_message(context, ENOMEM, N_("malloc: out of memory", ""));
    return ENOMEM;
}

void
_krb5_debug_backtrace(krb5_context context)
{
#if defined(HAVE_BACKTRACE) && !defined(HEIMDAL_SMALLER)
    void *stack[128];
    char **strs = NULL;
    int i, frames = backtrace(stack, sizeof(stack) / sizeof(stack[0]));
    if (frames > 0)
	strs = backtrace_symbols(stack, frames);
    if (strs) {
	for (i = 0; i < frames; i++)
	    _krb5_debug(context, 10, "frame %d: %s", i, strs[i]);
	free(strs);
    }
#endif
}

krb5_error_code
_krb5_einval(krb5_context context, const char *func, unsigned long argn)
{
#ifndef HEIMDAL_SMALLER
    krb5_set_error_message(context, EINVAL,
			   N_("programmer error: invalid argument to %s argument %lu",
			      "function:line"),
			   func, argn);
    if (_krb5_have_debug(context, 10)) {
	_krb5_debug(context, 10, "invalid argument to function %s argument %lu",
		    func, argn);
	_krb5_debug_backtrace(context);
    }
#endif
    return EINVAL;
}