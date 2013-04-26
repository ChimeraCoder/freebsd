
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

#include "hx_locl.h"

/**
 * @page page_error Hx509 error reporting functions
 *
 * See the library functions here: @ref hx509_error
 */

struct hx509_error_data {
    hx509_error next;
    int code;
    char *msg;
};

static void
free_error_string(hx509_error msg)
{
    while(msg) {
	hx509_error m2 = msg->next;
	free(msg->msg);
	free(msg);
	msg = m2;
    }
}

/**
 * Resets the error strings the hx509 context.
 *
 * @param context A hx509 context.
 *
 * @ingroup hx509_error
 */

void
hx509_clear_error_string(hx509_context context)
{
    if (context) {
	free_error_string(context->error);
	context->error = NULL;
    }
}

/**
 * Add an error message to the hx509 context.
 *
 * @param context A hx509 context.
 * @param flags
 * - HX509_ERROR_APPEND appends the error string to the old messages
     (code is updated).
 * @param code error code related to error message
 * @param fmt error message format
 * @param ap arguments to error message format
 *
 * @ingroup hx509_error
 */

void
hx509_set_error_stringv(hx509_context context, int flags, int code,
			const char *fmt, va_list ap)
{
    hx509_error msg;

    if (context == NULL)
	return;

    msg = calloc(1, sizeof(*msg));
    if (msg == NULL) {
	hx509_clear_error_string(context);
	return;
    }

    if (vasprintf(&msg->msg, fmt, ap) == -1) {
	hx509_clear_error_string(context);
	free(msg);
	return;
    }
    msg->code = code;

    if (flags & HX509_ERROR_APPEND) {
	msg->next = context->error;
	context->error = msg;
    } else  {
	free_error_string(context->error);
	context->error = msg;
    }
}

/**
 * See hx509_set_error_stringv().
 *
 * @param context A hx509 context.
 * @param flags
 * - HX509_ERROR_APPEND appends the error string to the old messages
     (code is updated).
 * @param code error code related to error message
 * @param fmt error message format
 * @param ... arguments to error message format
 *
 * @ingroup hx509_error
 */

void
hx509_set_error_string(hx509_context context, int flags, int code,
		       const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    hx509_set_error_stringv(context, flags, code, fmt, ap);
    va_end(ap);
}

/**
 * Get an error string from context associated with error_code.
 *
 * @param context A hx509 context.
 * @param error_code Get error message for this error code.
 *
 * @return error string, free with hx509_free_error_string().
 *
 * @ingroup hx509_error
 */

char *
hx509_get_error_string(hx509_context context, int error_code)
{
    struct rk_strpool *p = NULL;
    hx509_error msg = context->error;

    if (msg == NULL || msg->code != error_code) {
	const char *cstr;
	char *str;

	cstr = com_right(context->et_list, error_code);
	if (cstr)
	    return strdup(cstr);
	cstr = strerror(error_code);
	if (cstr)
	    return strdup(cstr);
	if (asprintf(&str, "<unknown error: %d>", error_code) == -1)
	    return NULL;
	return str;
    }

    for (msg = context->error; msg; msg = msg->next)
	p = rk_strpoolprintf(p, "%s%s", msg->msg,
			     msg->next != NULL ? "; " : "");

    return rk_strpoolcollect(p);
}

/**
 * Free error string returned by hx509_get_error_string().
 *
 * @param str error string to free.
 *
 * @ingroup hx509_error
 */

void
hx509_free_error_string(char *str)
{
    free(str);
}

/**
 * Print error message and fatally exit from error code
 *
 * @param context A hx509 context.
 * @param exit_code exit() code from process.
 * @param error_code Error code for the reason to exit.
 * @param fmt format string with the exit message.
 * @param ... argument to format string.
 *
 * @ingroup hx509_error
 */

void
hx509_err(hx509_context context, int exit_code,
	  int error_code, const char *fmt, ...)
{
    va_list ap;
    const char *msg;
    char *str;

    va_start(ap, fmt);
    vasprintf(&str, fmt, ap);
    va_end(ap);
    msg = hx509_get_error_string(context, error_code);
    if (msg == NULL)
	msg = "no error";

    errx(exit_code, "%s: %s", str, msg);
}