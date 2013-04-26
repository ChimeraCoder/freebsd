
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)getent.c	8.2 (Berkeley) 12/15/93";
#endif
#endif /* not lint */

#include <stdlib.h>
#include <string.h>

#include "misc-proto.h"

static char *area;
static char gettytab[] = "/etc/gettytab";

/*ARGSUSED*/
int
getent(char *cp __unused, const char *name)
{
	int retval;
	char *tempnam, *dba[2] = { gettytab, NULL };

	tempnam = strdup(name);
	retval =  cgetent(&area, dba, tempnam) == 0 ? 1 : 0;
	free(tempnam);
	return(retval);
}

/*ARGSUSED*/
char *
Getstr(const char *id, char **cpp __unused)
{
	int retval;
	char *answer, *tempid;

	tempid = strdup(id);
	retval = cgetstr(area, tempid, &answer);
	free(tempid);
	return((retval > 0) ? answer : NULL);
}