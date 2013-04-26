
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
#include <stdio.h>

#include "ngctl.h"

static int ConnectCmd(int ac, char **av);

const struct ngcmd connect_cmd = {
	ConnectCmd,
	"connect [path] <relpath> <hook> <peerhook>",
	"Connects hook <peerhook> of the node at <relpath> to <hook>",
	"The connect command creates a link between the two nodes at"
	" \"path\" and \"relpath\" using hooks \"hook\" and \"peerhook\","
	" respectively. The \"relpath\", if not absolute, is specified"
	" relative to the node at \"path\"."
	" If \"path\" is omitted then \".\" is assumed.",
	{ "join" }
};

static int
ConnectCmd(int ac, char **av)
{
	struct ngm_connect con;
	const char *path = ".";

	/* Get arguments */
	switch (ac) {
	case 5:
		path = av[1];
		ac--;
		av++;
		/* FALLTHROUGH */
	case 4:
		snprintf(con.path, sizeof(con.path), "%s", av[1]);
		snprintf(con.ourhook, sizeof(con.ourhook), "%s", av[2]);
		snprintf(con.peerhook, sizeof(con.peerhook), "%s", av[3]);
		break;
	default:
		return (CMDRTN_USAGE);
	}

	/* Send message */
	if (NgSendMsg(csock, path, NGM_GENERIC_COOKIE,
	    NGM_CONNECT, &con, sizeof(con)) < 0) {
		warn("send msg");
		return (CMDRTN_ERROR);
	}
	return (CMDRTN_OK);
}