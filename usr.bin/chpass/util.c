
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)util.c	8.4 (Berkeley) 4/2/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "chpass.h"

static const char *months[] =
	{ "January", "February", "March", "April", "May", "June",
	  "July", "August", "September", "October", "November",
	  "December", NULL };

char *
ttoa(time_t tval)
{
	struct tm *tp;
	static char tbuf[50];

	if (tval) {
		tp = localtime(&tval);
		(void)sprintf(tbuf, "%s %d, %d", months[tp->tm_mon],
		    tp->tm_mday, tp->tm_year + 1900);
	}
	else
		*tbuf = '\0';
	return (tbuf);
}

int
atot(char *p, time_t *store)
{
	static struct tm *lt;
	char *t;
	const char **mp;
	time_t tval;
	int day, month, year;

	if (!*p) {
		*store = 0;
		return (0);
	}
	if (!lt) {
		unsetenv("TZ");
		(void)time(&tval);
		lt = localtime(&tval);
	}
	if (!(t = strtok(p, " \t")))
		goto bad;
	if (isdigit(*t)) {
		month = atoi(t);
	} else {
		for (mp = months;; ++mp) {
			if (!*mp)
				goto bad;
			if (!strncasecmp(*mp, t, 3)) {
				month = mp - months + 1;
				break;
			}
		}
	}
	if (!(t = strtok(NULL, " \t,")) || !isdigit(*t))
		goto bad;
	day = atoi(t);
	if (!(t = strtok(NULL, " \t,")) || !isdigit(*t))
		goto bad;
	year = atoi(t);
	if (day < 1 || day > 31 || month < 1 || month > 12)
		goto bad;
	/* Allow two digit years 1969-2068 */
	if (year < 69)
		year += 2000;
	else if (year < 100)
		year += 1900;
	if (year < 1969)
bad:		return (1);
	lt->tm_year = year - 1900;
	lt->tm_mon = month - 1;
	lt->tm_mday = day;
	lt->tm_hour = 0;
	lt->tm_min = 0;
	lt->tm_sec = 0;
	lt->tm_isdst = -1;
	if ((tval = mktime(lt)) < 0)
		return (1);
	*store = tval;
	return (0);
}

int
ok_shell(char *name)
{
	char *p, *sh;

	setusershell();
	while ((sh = getusershell())) {
		if (!strcmp(name, sh)) {
			endusershell();
			return (1);
		}
		/* allow just shell name, but use "real" path */
		if ((p = strrchr(sh, '/')) && strcmp(name, p + 1) == 0) {
			endusershell();
			return (1);
		}
	}
	endusershell();
	return (0);
}

char *
dup_shell(char *name)
{
	char *p, *sh, *ret;

	setusershell();
	while ((sh = getusershell())) {
		if (!strcmp(name, sh)) {
			endusershell();
			return (strdup(name));
		}
		/* allow just shell name, but use "real" path */
		if ((p = strrchr(sh, '/')) && strcmp(name, p + 1) == 0) {
			ret = strdup(sh);
			endusershell();
			return (ret);
		}
	}
	endusershell();
	return (NULL);
}