
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
 /*
  * This module intercepts syslog() library calls and redirects their output
  * to the standard output stream. For interactive testing.
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */
#ifndef lint
static char sccsid[] = "@(#) fakelog.c 1.3 94/12/28 17:42:21";
#endif

#include <stdio.h>

#include "mystdarg.h"

/* openlog - dummy */

/* ARGSUSED */

openlog(name, logopt, facility)
char   *name;
int     logopt;
int     facility;
{
    /* void */
}

/* vsyslog - format one record */

vsyslog(severity, fmt, ap)
int     severity;
char   *fmt;
va_list ap;
{
    char    buf[BUFSIZ];

    vprintf(percent_m(buf, fmt), ap);
    printf("\n");
    fflush(stdout);
}

/* syslog - format one record */

/* VARARGS */

VARARGS(syslog, int, severity)
{
    va_list ap;
    char   *fmt;

    VASTART(ap, int, severity);
    fmt = va_arg(ap, char *);
    vsyslog(severity, fmt, ap);
    VAEND(ap);
}

/* closelog - dummy */

closelog()
{
    /* void */
}