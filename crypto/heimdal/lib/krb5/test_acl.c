
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
#include <err.h>

#define RETVAL(c, r, e, s) \
	do { if (r != e) krb5_errx(c, 1, "%s", s); } while (0)
#define STRINGMATCH(c, s, _s1, _s2) \
	do {							\
		if (_s1 == NULL || _s2 == NULL) 		\
			krb5_errx(c, 1, "s1 or s2 is NULL");	\
		if (strcmp(_s1,_s2) != 0) 			\
			krb5_errx(c, 1, "%s", s);		\
	} while (0)

static void
test_match_string(krb5_context context)
{
    krb5_error_code ret;
    char *s1, *s2;

    ret = krb5_acl_match_string(context, "foo", "s", "foo");
    RETVAL(context, ret, 0, "single s");
    ret = krb5_acl_match_string(context, "foo foo", "s", "foo");
    RETVAL(context, ret, EACCES, "too many strings");
    ret = krb5_acl_match_string(context, "foo bar", "ss", "foo", "bar");
    RETVAL(context, ret, 0, "two strings");
    ret = krb5_acl_match_string(context, "foo  bar", "ss", "foo", "bar");
    RETVAL(context, ret, 0, "two strings double space");
    ret = krb5_acl_match_string(context, "foo \tbar", "ss", "foo", "bar");
    RETVAL(context, ret, 0, "two strings space + tab");
    ret = krb5_acl_match_string(context, "foo", "ss", "foo", "bar");
    RETVAL(context, ret, EACCES, "one string, two format strings");
    ret = krb5_acl_match_string(context, "foo", "ss", "foo", "foo");
    RETVAL(context, ret, EACCES, "one string, two format strings (same)");
    ret = krb5_acl_match_string(context, "foo  \t", "s", "foo");
    RETVAL(context, ret, 0, "ending space");

    ret = krb5_acl_match_string(context, "foo/bar", "f", "foo/bar");
    RETVAL(context, ret, 0, "liternal fnmatch");
    ret = krb5_acl_match_string(context, "foo/bar", "f", "foo/*");
    RETVAL(context, ret, 0, "foo/*");
    ret = krb5_acl_match_string(context, "foo/bar.example.org", "f",
				"foo/*.example.org");
    RETVAL(context, ret, 0, "foo/*.example.org");
    ret = krb5_acl_match_string(context, "foo/bar.example.com", "f",
				"foo/*.example.org");
    RETVAL(context, ret, EACCES, "foo/*.example.com");

    ret = krb5_acl_match_string(context, "foo/bar/baz", "f", "foo/*/baz");
    RETVAL(context, ret, 0, "foo/*/baz");

    ret = krb5_acl_match_string(context, "foo", "r", &s1);
    RETVAL(context, ret, 0, "ret 1");
    STRINGMATCH(context, "ret 1 match", s1, "foo"); free(s1);

    ret = krb5_acl_match_string(context, "foo bar", "rr", &s1, &s2);
    RETVAL(context, ret, 0, "ret 2");
    STRINGMATCH(context, "ret 2 match 1", s1, "foo"); free(s1);
    STRINGMATCH(context, "ret 2 match 2", s2, "bar"); free(s2);

    ret = krb5_acl_match_string(context, "foo bar", "sr", "bar", &s1);
    RETVAL(context, ret, EACCES, "ret mismatch");
    if (s1 != NULL) krb5_errx(context, 1, "s1 not NULL");

    ret = krb5_acl_match_string(context, "foo", "l", "foo");
    RETVAL(context, ret, EINVAL, "unknown letter");
}


int
main(int argc, char **argv)
{
    krb5_context context;
    krb5_error_code ret;

    setprogname(argv[0]);

    ret = krb5_init_context(&context);
    if (ret)
	errx (1, "krb5_init_context failed: %d", ret);

    test_match_string(context);

    krb5_free_context(context);

    return 0;
}