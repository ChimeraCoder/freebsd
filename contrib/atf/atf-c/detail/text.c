
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

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "atf-c/error.h"

#include "dynstr.h"
#include "sanity.h"
#include "text.h"

atf_error_t
atf_text_for_each_word(const char *instr, const char *sep,
                       atf_error_t (*func)(const char *, void *),
                       void *data)
{
    atf_error_t err;
    char *str, *str2, *last;

    str = strdup(instr);
    if (str == NULL) {
        err = atf_no_memory_error();
        goto out;
    }

    err = atf_no_error();
    str2 = strtok_r(str, sep, &last);
    while (str2 != NULL && !atf_is_error(err)) {
        err = func(str2, data);
        str2 = strtok_r(NULL, sep, &last);
    }

    free(str);
out:
    return err;
}

atf_error_t
atf_text_format(char **dest, const char *fmt, ...)
{
    atf_error_t err;
    va_list ap;

    va_start(ap, fmt);
    err = atf_text_format_ap(dest, fmt, ap);
    va_end(ap);

    return err;
}

atf_error_t
atf_text_format_ap(char **dest, const char *fmt, va_list ap)
{
    atf_error_t err;
    atf_dynstr_t tmp;
    va_list ap2;

    va_copy(ap2, ap);
    err = atf_dynstr_init_ap(&tmp, fmt, ap2);
    va_end(ap2);
    if (!atf_is_error(err))
        *dest = atf_dynstr_fini_disown(&tmp);

    return err;
}

atf_error_t
atf_text_split(const char *str, const char *delim, atf_list_t *words)
{
    atf_error_t err;
    const char *end;
    const char *iter;

    err = atf_list_init(words);
    if (atf_is_error(err))
        goto err;

    end = str + strlen(str);
    INV(*end == '\0');
    iter = str;
    while (iter < end) {
        const char *ptr;

        INV(iter != NULL);
        ptr = strstr(iter, delim);
        if (ptr == NULL)
            ptr = end;

        INV(ptr >= iter);
        if (ptr > iter) {
            atf_dynstr_t word;

            err = atf_dynstr_init_raw(&word, iter, ptr - iter);
            if (atf_is_error(err))
                goto err_list;

            err = atf_list_append(words, atf_dynstr_fini_disown(&word), true);
            if (atf_is_error(err))
                goto err_list;
        }

        iter = ptr + strlen(delim);
    }

    INV(!atf_is_error(err));
    return err;

err_list:
    atf_list_fini(words);
err:
    return err;
}

atf_error_t
atf_text_to_bool(const char *str, bool *b)
{
    atf_error_t err;

    if (strcasecmp(str, "yes") == 0 ||
        strcasecmp(str, "true") == 0) {
        *b = true;
        err = atf_no_error();
    } else if (strcasecmp(str, "no") == 0 ||
               strcasecmp(str, "false") == 0) {
        *b = false;
        err = atf_no_error();
    } else {
        /* XXX Not really a libc error. */
        err = atf_libc_error(EINVAL, "Cannot convert string '%s' "
                             "to boolean", str);
    }

    return err;
}

atf_error_t
atf_text_to_long(const char *str, long *l)
{
    atf_error_t err;
    char *endptr;
    long tmp;

    errno = 0;
    tmp = strtol(str, &endptr, 10);
    if (str[0] == '\0' || *endptr != '\0')
        err = atf_libc_error(EINVAL, "'%s' is not a number", str);
    else if (errno == ERANGE || (tmp == LONG_MAX || tmp == LONG_MIN))
        err = atf_libc_error(ERANGE, "'%s' is out of range", str);
    else {
        *l = tmp;
        err = atf_no_error();
    }

    return err;
}