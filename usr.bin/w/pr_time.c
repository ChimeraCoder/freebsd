
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
static const char sccsid[] = "@(#)pr_time.c	8.2 (Berkeley) 4/4/94";
#endif

#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "extern.h"

/*
 * pr_attime --
 *	Print the time since the user logged in.
 */
int
pr_attime(time_t *started, time_t *now)
{
	static wchar_t buf[256];
	struct tm tp, tm;
	time_t diff;
	const wchar_t *fmt;
	int len, width, offset = 0;

	tp = *localtime(started);
	tm = *localtime(now);
	diff = *now - *started;

	/* If more than a week, use day-month-year. */
	if (diff > 86400 * 7)
		fmt = L"%d%b%y";

	/* If not today, use day-hour-am/pm. */
	else if (tm.tm_mday != tp.tm_mday ||
		 tm.tm_mon != tp.tm_mon ||
		 tm.tm_year != tp.tm_year) {
	/* The line below does not take DST into consideration */
	/* else if (*now / 86400 != *started / 86400) { */
		fmt = use_ampm ? L"%a%I%p" : L"%a%H";
	}

	/* Default is hh:mm{am,pm}. */
	else {
		fmt = use_ampm ? L"%l:%M%p" : L"%k:%M";
	}

	(void)wcsftime(buf, sizeof(buf), fmt, &tp);
	len = wcslen(buf);
	width = wcswidth(buf, len);
	if (len == width)
		(void)wprintf(L"%-7.7ls", buf);
	else if (width < 7)
		(void)wprintf(L"%ls%.*s", buf, 7 - width, "      ");
	else {
		(void)wprintf(L"%ls", buf);
		offset = width - 7;
	}
	return (offset);
}

/*
 * pr_idle --
 *	Display the idle time.
 *	Returns number of excess characters that were used for long idle time.
 */
int
pr_idle(time_t idle)
{
	/* If idle more than 36 hours, print as a number of days. */
	if (idle >= 36 * 3600) {
		int days = idle / 86400;
		(void)printf(" %dday%s ", days, days > 1 ? "s" : " " );
		if (days >= 100)
			return (2);
		if (days >= 10)
			return (1);
	}

	/* If idle more than an hour, print as HH:MM. */
	else if (idle >= 3600)
		(void)printf(" %2d:%02d ",
		    (int)(idle / 3600), (int)((idle % 3600) / 60));

	else if (idle / 60 == 0)
		(void)printf("     - ");

	/* Else print the minutes idle. */
	else
		(void)printf("    %2d ", (int)(idle / 60));

	return (0); /* not idle longer than 9 days */
}