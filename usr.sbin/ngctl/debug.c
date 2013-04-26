
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

#include <netgraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ngctl.h"

static int DebugCmd(int ac, char **av);

const struct ngcmd debug_cmd = {
	DebugCmd,
	"debug [level]",
	"Get/set debugging verbosity level",
	"Without any argument, this command displays the current"
	" debugging verbosity level. If the argument is ``+'' or ``-''"
	" the debug level is incremented or decremented; otherwise,"
	" it must be an absolute numerical level.",
	{ NULL }
};

static int
DebugCmd(int ac, char **av)
{
	int level;

	/* Get arguments */
	switch (ac) {
	case 2:
		if (!strcmp(av[1], "+"))
			level = NgSetDebug(-1) + 1;
		else if (!strcmp(av[1], "-"))
			level = NgSetDebug(-1) - 1;
		else if ((level = atoi(av[1])) < 0)
			return (CMDRTN_USAGE);
		NgSetDebug(level);
		break;
	case 1:
		printf("Current debug level is %d\n", NgSetDebug(-1));
		break;
	default:
		return (CMDRTN_USAGE);
	}
	return (CMDRTN_OK);
}