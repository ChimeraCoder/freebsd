
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
  * vsyslog() for sites without. In order to enable this code, build with
  * -Dvsyslog=myvsyslog. We use a different name so that no accidents will
  * happen when vsyslog() exists. On systems with vsyslog(), syslog() is
  * typically implemented in terms of vsyslog().
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */
#ifndef lint
static char sccsid[] = "@(#) myvsyslog.c 1.1 94/12/28 17:42:33";
#endif

#ifdef vsyslog

#include <stdio.h>

#include "tcpd.h"
#include "mystdarg.h"

myvsyslog(severity, format, ap)
int     severity;
char   *format;
va_list ap;
{
    char    fbuf[BUFSIZ];
    char    obuf[3 * STRING_LENGTH];

    vsprintf(obuf, percent_m(fbuf, format), ap);
    syslog(severity, "%s", obuf);
}

#endif