
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

#undef __attribute__
#define __attribute__(x)

/**
 * Clears the error message from the Kerberos 5 context.
 *
 * @param context The Kerberos 5 context to clear
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_clear_error_message(krb5_context context)
{
    HEIMDAL_MUTEX_lock(context->mutex);
    if (context->error_string)
	free(context->error_string);
    context->error_code = 0;
    context->error_string = NULL;
    HEIMDAL_MUTEX_unlock(context->mutex);
}

/**
 * Set the context full error string for a specific error code.
 * The error that is stored should be internationalized.
 *
 * The if context is NULL, no error string is stored.
 *
 * @param context Kerberos 5 context
 * @param ret The error code
 * @param fmt Error string for the error code
 * @param ... printf(3) style parameters.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_set_error_message(krb5_context context, krb5_error_code ret,
		       const char *fmt, ...)
    __attribute__ ((format (printf, 3, 4)))
{
    va_list ap;

    va_start(ap, fmt);
    krb5_vset_error_message (context, ret, fmt, ap);
    va_end(ap);
}

/**
 * Set the context full error string for a specific error code.
 *
 * The if context is NULL, no error string is stored.
 *
 * @param context Kerberos 5 context
 * @param ret The error code
 * @param fmt Error string for the error code
 * @param args printf(3) style parameters.
 *
 * @ingroup krb5_error
 */


KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_vset_error_message (krb5_context context, krb5_error_code ret,
			 const char *fmt, va_list args)
    __attribute__ ((format (printf, 3, 0)))
{
    int r;

    if (context == NULL)
	return;

    HEIMDAL_MUTEX_lock(context->mutex);
    if (context->error_string) {
	free(context->error_string);
	context->error_string = NULL;
    }
    context->error_code = ret;
    r = vasprintf(&context->error_string, fmt, args);
    if (r < 0)
	context->error_string = NULL;
    HEIMDAL_MUTEX_unlock(context->mutex);
}

/**
 * Prepend the context full error string for a specific error code.
 * The error that is stored should be internationalized.
 *
 * The if context is NULL, no error string is stored.
 *
 * @param context Kerberos 5 context
 * @param ret The error code
 * @param fmt Error string for the error code
 * @param ... printf(3) style parameters.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_prepend_error_message(krb5_context context, krb5_error_code ret,
			   const char *fmt, ...)
    __attribute__ ((format (printf, 3, 4)))
{
    va_list ap;

    va_start(ap, fmt);
    krb5_vprepend_error_message(context, ret, fmt, ap);
    va_end(ap);
}

/**
 * Prepend the contexts's full error string for a specific error code.
 *
 * The if context is NULL, no error string is stored.
 *
 * @param context Kerberos 5 context
 * @param ret The error code
 * @param fmt Error string for the error code
 * @param args printf(3) style parameters.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_vprepend_error_message(krb5_context context, krb5_error_code ret,
			    const char *fmt, va_list args)
    __attribute__ ((format (printf, 3, 0)))
{
    char *str = NULL, *str2 = NULL;

    if (context == NULL)
	return;

    HEIMDAL_MUTEX_lock(context->mutex);
    if (context->error_code != ret) {
	HEIMDAL_MUTEX_unlock(context->mutex);
	return;
    }
    if (vasprintf(&str, fmt, args) < 0 || str == NULL) {
	HEIMDAL_MUTEX_unlock(context->mutex);
	return;
    }
    if (context->error_string) {
	int e;

	e = asprintf(&str2, "%s: %s", str, context->error_string);
	free(context->error_string);
	if (e < 0 || str2 == NULL)
	    context->error_string = NULL;
	else
	    context->error_string = str2;
	free(str);
    } else
	context->error_string = str;
    HEIMDAL_MUTEX_unlock(context->mutex);
}


/**
 * Return the error message in context. On error or no error string,
 * the function returns NULL.
 *
 * @param context Kerberos 5 context
 *
 * @return an error string, needs to be freed with
 * krb5_free_error_message(). The functions return NULL on error.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION char * KRB5_LIB_CALL
krb5_get_error_string(krb5_context context)
{
    char *ret = NULL;

    HEIMDAL_MUTEX_lock(context->mutex);
    if (context->error_string)
	ret = strdup(context->error_string);
    HEIMDAL_MUTEX_unlock(context->mutex);
    return ret;
}

KRB5_LIB_FUNCTION krb5_boolean KRB5_LIB_CALL
krb5_have_error_string(krb5_context context)
{
    char *str;
    HEIMDAL_MUTEX_lock(context->mutex);
    str = context->error_string;
    HEIMDAL_MUTEX_unlock(context->mutex);
    return str != NULL;
}

/**
 * Return the error message for `code' in context. On memory
 * allocation error the function returns NULL.
 *
 * @param context Kerberos 5 context
 * @param code Error code related to the error
 *
 * @return an error string, needs to be freed with
 * krb5_free_error_message(). The functions return NULL on error.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION const char * KRB5_LIB_CALL
krb5_get_error_message(krb5_context context, krb5_error_code code)
{
    char *str = NULL;
    const char *cstr = NULL;
    char buf[128];
    int free_context = 0;

    if (code == 0)
	return strdup("Success");

    /*
     * The MIT version of this function ignores the krb5_context
     * and several widely deployed applications call krb5_get_error_message()
     * with a NULL context in order to translate an error code as a
     * replacement for error_message().  Another reason a NULL context
     * might be provided is if the krb5_init_context() call itself
     * failed.
     */
    if (context)
    {
        HEIMDAL_MUTEX_lock(context->mutex);
        if (context->error_string &&
            (code == context->error_code || context->error_code == 0))
        {
            str = strdup(context->error_string);
        }
        HEIMDAL_MUTEX_unlock(context->mutex);

        if (str)
            return str;
    }
    else
    {
        if (krb5_init_context(&context) == 0)
            free_context = 1;
    }

    if (context)
        cstr = com_right_r(context->et_list, code, buf, sizeof(buf));

    if (free_context)
        krb5_free_context(context);

    if (cstr)
        return strdup(cstr);

    cstr = error_message(code);
    if (cstr)
        return strdup(cstr);

    if (asprintf(&str, "<unknown error: %d>", (int)code) == -1 || str == NULL)
	return NULL;

    return str;
}


/**
 * Free the error message returned by krb5_get_error_message().
 *
 * @param context Kerberos context
 * @param msg error message to free, returned byg
 *        krb5_get_error_message().
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_error_message(krb5_context context, const char *msg)
{
    free(rk_UNCONST(msg));
}


/**
 * Return the error string for the error code. The caller must not
 * free the string.
 *
 * This function is deprecated since its not threadsafe.
 *
 * @param context Kerberos 5 context.
 * @param code Kerberos error code.
 *
 * @return the error message matching code
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION const char* KRB5_LIB_CALL
krb5_get_err_text(krb5_context context, krb5_error_code code)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    const char *p = NULL;
    if(context != NULL)
	p = com_right(context->et_list, code);
    if(p == NULL)
	p = strerror(code);
    if (p == NULL)
	p = "Unknown error";
    return p;
}