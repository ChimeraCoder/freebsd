
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

#include <err.h>
#include <netgraph.h>
#include <unistd.h>

#include "ngctl.h"

static int ShutdownCmd(int ac, char **av);

const struct ngcmd shutdown_cmd = {
	ShutdownCmd,
	"shutdown <path>",
	"Shutdown the node at <path>",
	NULL,
	{ "kill", "rmnode" }
};

static int
ShutdownCmd(int ac, char **av)
{
	char *path;

	/* Get arguments */
	switch (ac) {
	case 2:
		path = av[1];
		break;
	default:
		return (CMDRTN_USAGE);
	}

	/* Shutdown node */
	if (NgSendMsg(csock, path, NGM_GENERIC_COOKIE,
	    NGM_SHUTDOWN, NULL, 0) < 0) {
		warn("shutdown");
		return (CMDRTN_ERROR);
	}
	return (CMDRTN_OK);
}