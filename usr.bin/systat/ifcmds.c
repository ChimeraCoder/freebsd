
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

#include <sys/types.h>

#include "systat.h"
#include "extern.h"
#include "convtbl.h"

#include <stdlib.h>
#include <string.h>

int curscale = SC_AUTO;
char *matchline = NULL;
int showpps = 0;
int needsort = 0;

int
ifcmd(const char *cmd, const char *args)
{
	int scale;

	if (prefix(cmd, "scale")) {
		if ((scale = get_scale(args)) != -1)
			curscale = scale;
		else {
			move(CMDLINE, 0);
			clrtoeol();
			addstr("what scale? ");
			addstr(get_helplist());
		}
	} else if (prefix(cmd, "match")) {
		if (args != NULL && *args != '\0' && memcmp(args, "*", 2) != 0) {
			/* We got a valid match line */
			if (matchline != NULL)
				free(matchline);
			needsort = 1;
			matchline = strdup(args);
		} else {
			/* Empty or * pattern, turn filtering off */
			if (matchline != NULL)
				free(matchline);
			needsort = 1;
			matchline = NULL;
		}
	} else if (prefix(cmd, "pps"))
		showpps = !showpps;

	return (1);
}