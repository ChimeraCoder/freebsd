
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

#include <config.h>

#ifndef HAVE_VSYSLOG

#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>

#include "roken.h"

/*
 * the theory behind this is that we might be trying to call vsyslog
 * when there's no memory left, and we should try to be as useful as
 * possible.  And the format string should say something about what's
 * failing.
 */

static void
simple_vsyslog(int pri, const char *fmt, va_list ap)
{
    syslog (pri, "%s", fmt);
}

/*
 * do like syslog but with a `va_list'
 */

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
vsyslog(int pri, const char *fmt, va_list ap)
{
    char *fmt2;
    const char *p;
    char *p2;
    int ret;
    int saved_errno = errno;
    int fmt_len  = strlen (fmt);
    int fmt2_len = fmt_len;
    char *buf;

    fmt2 = malloc (fmt_len + 1);
    if (fmt2 == NULL) {
	simple_vsyslog (pri, fmt, ap);
	return;
    }

    for (p = fmt, p2 = fmt2; *p != '\0'; ++p) {
	if (p[0] == '%' && p[1] == 'm') {
	    const char *e = strerror (saved_errno);
	    int e_len = strlen (e);
	    char *tmp;
	    int pos;

	    pos = p2 - fmt2;
	    fmt2_len += e_len - 2;
	    tmp = realloc (fmt2, fmt2_len + 1);
	    if (tmp == NULL) {
		free (fmt2);
		simple_vsyslog (pri, fmt, ap);
		return;
	    }
	    fmt2 = tmp;
	    p2   = fmt2 + pos;
	    memmove (p2, e, e_len);
	    p2 += e_len;
	    ++p;
	} else
	    *p2++ = *p;
    }
    *p2 = '\0';

    ret = vasprintf (&buf, fmt2, ap);
    free (fmt2);
    if (ret < 0 || buf == NULL) {
	simple_vsyslog (pri, fmt, ap);
	return;
    }
    syslog (pri, "%s", buf);
    free (buf);
}
#endif