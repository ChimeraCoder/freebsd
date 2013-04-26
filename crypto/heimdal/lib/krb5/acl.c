
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
#include <fnmatch.h>

struct acl_field {
    enum { acl_string, acl_fnmatch, acl_retval } type;
    union {
	const char *cstr;
	char **retv;
    } u;
    struct acl_field *next, **last;
};

static void
free_retv(struct acl_field *acl)
{
    while(acl != NULL) {
	if (acl->type == acl_retval) {
	    if (*acl->u.retv)
		free(*acl->u.retv);
	    *acl->u.retv = NULL;
	}
	acl = acl->next;
    }
}

static void
acl_free_list(struct acl_field *acl, int retv)
{
    struct acl_field *next;
    if (retv)
	free_retv(acl);
    while(acl != NULL) {
	next = acl->next;
	free(acl);
	acl = next;
    }
}

static krb5_error_code
acl_parse_format(krb5_context context,
		 struct acl_field **acl_ret,
		 const char *format,
		 va_list ap)
{
    const char *p;
    struct acl_field *acl = NULL, *tmp;

    for(p = format; *p != '\0'; p++) {
	tmp = malloc(sizeof(*tmp));
	if(tmp == NULL) {
	    krb5_set_error_message(context, ENOMEM,
				   N_("malloc: out of memory", ""));
	    acl_free_list(acl, 0);
	    return ENOMEM;
	}
	if(*p == 's') {
	    tmp->type = acl_string;
	    tmp->u.cstr = va_arg(ap, const char*);
	} else if(*p == 'f') {
	    tmp->type = acl_fnmatch;
	    tmp->u.cstr = va_arg(ap, const char*);
	} else if(*p == 'r') {
	    tmp->type = acl_retval;
	    tmp->u.retv = va_arg(ap, char **);
	    *tmp->u.retv = NULL;
	} else {
	    krb5_set_error_message(context, EINVAL,
				   N_("Unknown format specifier %c while "
				     "parsing ACL", "specifier"), *p);
	    acl_free_list(acl, 0);
	    free(tmp);
	    return EINVAL;
	}
	tmp->next = NULL;
	if(acl == NULL)
	    acl = tmp;
	else
	    *acl->last = tmp;
	acl->last = &tmp->next;
    }
    *acl_ret = acl;
    return 0;
}

static krb5_boolean
acl_match_field(krb5_context context,
		const char *string,
		struct acl_field *field)
{
    if(field->type == acl_string) {
	return !strcmp(field->u.cstr, string);
    } else if(field->type == acl_fnmatch) {
	return !fnmatch(field->u.cstr, string, 0);
    } else if(field->type == acl_retval) {
	*field->u.retv = strdup(string);
	return TRUE;
    }
    return FALSE;
}

static krb5_boolean
acl_match_acl(krb5_context context,
	      struct acl_field *acl,
	      const char *string)
{
    char buf[256];
    while(strsep_copy(&string, " \t", buf, sizeof(buf)) != -1) {
	if(buf[0] == '\0')
	    continue; /* skip ws */
	if (acl == NULL)
	    return FALSE;
	if(!acl_match_field(context, buf, acl)) {
	    return FALSE;
	}
	acl = acl->next;
    }
    if (acl)
	return FALSE;
    return TRUE;
}

/**
 * krb5_acl_match_string matches ACL format against a string.
 *
 * The ACL format has three format specifiers: s, f, and r.  Each
 * specifier will retrieve one argument from the variable arguments
 * for either matching or storing data.  The input string is split up
 * using " " (space) and "\t" (tab) as a delimiter; multiple and "\t"
 * in a row are considered to be the same.
 *
 * List of format specifiers:
 * - s Matches a string using strcmp(3) (case sensitive).
 * - f Matches the string with fnmatch(3). Theflags
 *     argument (the last argument) passed to the fnmatch function is 0.
 * - r Returns a copy of the string in the char ** passed in; the copy
 *     must be freed with free(3). There is no need to free(3) the
 *     string on error: the function will clean up and set the pointer
 *     to NULL.
 *
 * @param context Kerberos 5 context
 * @param string string to match with
 * @param format format to match
 * @param ... parameter to format string
 *
 * @return Return an error code or 0.
 *
 *
 * @code
 * char *s;
 *
 * ret = krb5_acl_match_string(context, "foo", "s", "foo");
 * if (ret)
 *     krb5_errx(context, 1, "acl didn't match");
 * ret = krb5_acl_match_string(context, "foo foo baz/kaka",
 *     "ss", "foo", &s, "foo/\\*");
 * if (ret) {
 *     // no need to free(s) on error
 *     assert(s == NULL);
 *     krb5_errx(context, 1, "acl didn't match");
 * }
 * free(s);
 * @endcode
 *
 * @sa krb5_acl_match_file
 * @ingroup krb5_support
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_acl_match_string(krb5_context context,
		      const char *string,
		      const char *format,
		      ...)
{
    krb5_error_code ret;
    krb5_boolean found;
    struct acl_field *acl;

    va_list ap;
    va_start(ap, format);
    ret = acl_parse_format(context, &acl, format, ap);
    va_end(ap);
    if(ret)
	return ret;

    found = acl_match_acl(context, acl, string);
    acl_free_list(acl, !found);
    if (found) {
	return 0;
    } else {
	krb5_set_error_message(context, EACCES, N_("ACL did not match", ""));
	return EACCES;
    }
}

/**
 * krb5_acl_match_file matches ACL format against each line in a file
 * using krb5_acl_match_string(). Lines starting with # are treated
 * like comments and ignored.
 *
 * @param context Kerberos 5 context.
 * @param file file with acl listed in the file.
 * @param format format to match.
 * @param ... parameter to format string.
 *
 * @return Return an error code or 0.
 *
 * @sa krb5_acl_match_string
 * @ingroup krb5_support
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_acl_match_file(krb5_context context,
		    const char *file,
		    const char *format,
		    ...)
{
    krb5_error_code ret;
    struct acl_field *acl;
    char buf[256];
    va_list ap;
    FILE *f;
    krb5_boolean found;

    f = fopen(file, "r");
    if(f == NULL) {
	int save_errno = errno;
	rk_strerror_r(save_errno, buf, sizeof(buf));
	krb5_set_error_message(context, save_errno,
			       N_("open(%s): %s", "file, errno"),
			       file, buf);
	return save_errno;
    }
    rk_cloexec_file(f);

    va_start(ap, format);
    ret = acl_parse_format(context, &acl, format, ap);
    va_end(ap);
    if(ret) {
	fclose(f);
	return ret;
    }

    found = FALSE;
    while(fgets(buf, sizeof(buf), f)) {
	if(buf[0] == '#')
	    continue;
	if(acl_match_acl(context, acl, buf)) {
	    found = TRUE;
	    break;
	}
	free_retv(acl);
    }

    fclose(f);
    acl_free_list(acl, !found);
    if (found) {
	return 0;
    } else {
	krb5_set_error_message(context, EACCES, N_("ACL did not match", ""));
	return EACCES;
    }
}