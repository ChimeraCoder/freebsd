
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

static int MkPeerCmd(int ac, char **av);

const struct ngcmd mkpeer_cmd = {
	MkPeerCmd,
	"mkpeer [path] <type> <hook> <peerhook>",
	"Create and connect a new node to the node at \"path\"",
	"The mkpeer command atomically creates a new node of type \"type\""
	" and connects it to the node at \"path\". The hooks used for the"
	" connection are \"hook\" on the original node and \"peerhook\""
	" on the new node."
	" If \"path\" is omitted then \".\" is assumed.",
	{ NULL }
};

static int
MkPeerCmd(int ac, char **av)
{
	struct ngm_mkpeer mkp;
	const char *path = ".";

	/* Get arguments */
	switch (ac) {
	case 5:
		path = av[1];
		ac--;
		av++;
		/* FALLTHROUGH */
	case 4:
		snprintf(mkp.type, sizeof(mkp.type), "%s", av[1]);
		snprintf(mkp.ourhook, sizeof(mkp.ourhook), "%s", av[2]);
		snprintf(mkp.peerhook, sizeof(mkp.peerhook), "%s", av[3]);
		break;
	default:
		return (CMDRTN_USAGE);
	}

	/* Send message */
	if (NgSendMsg(csock, path, NGM_GENERIC_COOKIE,
	    NGM_MKPEER, &mkp, sizeof(mkp)) < 0) {
		warn("send msg");
		return (CMDRTN_ERROR);
	}
	return (CMDRTN_OK);
}