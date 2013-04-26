
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
 * mode.c - mechanisms for dealing with SGI-style modal displays.
 *
 * There are four generally-understood useful modes for status displays
 * of the sort exemplified by the IRIX ``netstat -C'' and ``osview''
 * programs.  We try to follow their example, although the user interface
 * and terminology slightly differ.
 *
 * RATE - the default mode - displays the precise rate of change in
 * each statistic in units per second, regardless of the actual display
 * update interval.
 *
 * DELTA - displays the change in each statistic over the entire
 * display update interval (i.e., RATE * interval).
 *
 * SINCE - displays the total change in each statistic since the module
 * was last initialized or reset.
 *
 * ABSOLUTE - displays the current value of each statistic.
 *
 * In the SGI programs, these modes are selected by the single-character
 * commands D, W, N, and A.  In systat, they are the slightly-harder-to-type
 * ``mode delta'', etc.  The initial value for SINCE mode is initialized
 * when the module is first started and can be reset using the ``reset''
 * command (as opposed to the SGI way where changing modes implicitly
 * resets).  A ``mode'' command with no arguments displays the current
 * mode in the command line.
 */

#include <sys/cdefs.h>

__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include "systat.h"
#include "extern.h"
#include "mode.h"

enum mode currentmode = display_RATE;

static const char *const modes[] = { "rate", "delta", "since", "absolute" };

int
cmdmode(const char *cmd, const char *args)
{
	if (prefix(cmd, "mode")) {
		if (args[0] == '\0') {
			move(CMDLINE, 0);
			clrtoeol();
			printw("%s", modes[currentmode]);
		} else if (prefix(args, "rate")) {
			currentmode = display_RATE;
		} else if (prefix(args, "delta")) {
			currentmode = display_DELTA;
		} else if (prefix(args, "since")) {
			currentmode = display_SINCE;
		} else if (prefix(args, "absolute")) {
			currentmode = display_ABS;
		} else {
			printw("unknown mode `%s'", args);
		}
		return 1;
	}
	if(prefix(cmd, "reset")) {
		curcmd->c_reset();
		return 1;
	}
	return 0;
}