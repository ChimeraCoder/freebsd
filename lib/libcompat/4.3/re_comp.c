
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Compatibility routines that implement the old re_comp/re_exec interface in
 * terms of the regcomp/regexec interface.  It's possible that some programs
 * rely on dark corners of re_comp/re_exec and won't work with this version,
 * but most programs should be fine.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)regex.c	5.1 (Berkeley) 3/29/92";
#endif /* LIBC_SCCS and not lint */

#include <regex.h>
#include <stddef.h>
#include <unistd.h>

static regex_t re_regexp;
static int re_gotexp;
static char re_errstr[100];

char *
re_comp(const char *s)
{
	int rc;

	if (s == NULL || *s == '\0') {
		if (!re_gotexp)
			return __DECONST(char *,
			    "no previous regular expression");
		return (NULL);
	}

	if (re_gotexp) {
		regfree(&re_regexp);
		re_gotexp = 0;
	}

	rc = regcomp(&re_regexp, s, REG_EXTENDED);
	if (rc == 0) {
		re_gotexp = 1;
		return (NULL);
	}

	regerror(rc, &re_regexp, re_errstr, sizeof(re_errstr));
	re_errstr[sizeof(re_errstr) - 1] = '\0';
	return (re_errstr);
}

int
re_exec(const char *s)
{
	int rc;

	if (!re_gotexp)
		return (-1);
	rc = regexec(&re_regexp, s, 0, NULL, 0);
	return (rc == 0 ? 1 : 0);
}