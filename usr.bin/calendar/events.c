
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

#include <sys/time.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pathnames.h"
#include "calendar.h"

struct event *
event_add(int year, int month, int day, char *date, int var, char *txt,
    char *extra)
{
	struct event *e;

	/*
	 * Creating a new event:
	 * - Create a new event
	 * - Copy the machine readable day and month
	 * - Copy the human readable and language specific date
	 * - Copy the text of the event
	 */
	e = (struct event *)calloc(1, sizeof(struct event));
	if (e == NULL)
		errx(1, "event_add: cannot allocate memory");
	e->month = month;
	e->day = day;
	e->var = var;
	e->date = strdup(date);
	if (e->date == NULL)
		errx(1, "event_add: cannot allocate memory");
	e->text = strdup(txt);
	if (e->text == NULL)
		errx(1, "event_add: cannot allocate memory");
	e->extra = NULL;
	if (extra != NULL && extra[0] != '\0')
		e->extra = strdup(extra);
	addtodate(e, year, month, day);
	return (e);
}

void
event_continue(struct event *e, char *txt)
{
	char *text;

	/*
	 * Adding text to the event:
	 * - Save a copy of the old text (unknown length, so strdup())
	 * - Allocate enough space for old text + \n + new text + 0
	 * - Store the old text + \n + new text
	 * - Destroy the saved copy.
	 */
	text = strdup(e->text);
	if (text == NULL)
		errx(1, "event_continue: cannot allocate memory");

	free(e->text);
	e->text = (char *)malloc(strlen(text) + strlen(txt) + 3);
	if (e->text == NULL)
		errx(1, "event_continue: cannot allocate memory");
	strcpy(e->text, text);
	strcat(e->text, "\n");
	strcat(e->text, txt);
	free(text);

	return;
}

void
event_print_all(FILE *fp)
{
	struct event *e;

	while (walkthrough_dates(&e) != 0) {
#ifdef DEBUG
		fprintf(stderr, "event_print_allmonth: %d, day: %d\n",
		    month, day);
#endif

		/*
		 * Go through all events and print the text of the matching
		 * dates
		 */
		while (e != NULL) {
			(void)fprintf(fp, "%s%c%s%s%s%s\n", e->date,
			    e->var ? '*' : ' ', e->text,
			    e->extra != NULL ? " (" : "",
			    e->extra != NULL ? e->extra : "",
			    e->extra != NULL ? ")" : ""
			);

			e = e->next;
		}
	}
}