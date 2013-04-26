
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

#include "utils.h"

/*LINTLIBRARY*/

static const char *pname;

#pragma init(getpname)
const char *
getpname(void)
{
	const char *p, *q;

	if (pname != NULL)
		return (pname);

	if ((p = getexecname()) != NULL)
		q = strrchr(p, '/');
	else
		q = NULL;

	if (q == NULL)
		pname = p;
	else
		pname = q + 1;

	return (pname);
}

void
vwarn(const char *format, va_list alist)
{
	int err = errno;

	if (pname != NULL)
		(void) fprintf(stderr, "%s: ", pname);

	(void) vfprintf(stderr, format, alist);

	if (strchr(format, '\n') == NULL)
		(void) fprintf(stderr, ": %s\n", strerror(err));
}

/*PRINTFLIKE1*/
void
warn(const char *format, ...)
{
	va_list alist;

	va_start(alist, format);
	vwarn(format, alist);
	va_end(alist);
}

void
vdie(const char *format, va_list alist)
{
	vwarn(format, alist);
	exit(E_ERROR);
}

/*PRINTFLIKE1*/
void
die(const char *format, ...)
{
	va_list alist;

	va_start(alist, format);
	vdie(format, alist);
	va_end(alist);
}