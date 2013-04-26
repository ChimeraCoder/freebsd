
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
#include <stdio.h>
#include <unistd.h>
#include <netgraph.h>

#include "ngctl.h"

static int NameCmd(int ac, char **av);

const struct ngcmd name_cmd = {
	NameCmd,
	"name <path> <name>",
	"Assign name <name> to the node at <path>",
	NULL,
	{ NULL }
};

static int
NameCmd(int ac, char **av)
{
	struct ngm_name name;
	char *path;

	/* Get arguments */
	switch (ac) {
	case 3:
		path = av[1];
		snprintf(name.name, sizeof(name.name), "%s", av[2]);
		break;
	default:
		return (CMDRTN_USAGE);
	}

	/* Send message */
	if (NgSendMsg(csock, path, NGM_GENERIC_COOKIE,
	    NGM_NAME, &name, sizeof(name)) < 0) {
		warn("send msg");
		return (CMDRTN_ERROR);
	}
	return (CMDRTN_OK);
}