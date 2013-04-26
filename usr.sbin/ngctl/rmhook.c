
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

static int RmHookCmd(int ac, char **av);

const struct ngcmd rmhook_cmd = {
	RmHookCmd,
	"rmhook [path] <hook>",
	"Disconnect hook \"hook\" of the node at \"path\"",
	"The rmhook command forces the node at \"path\" to break the link"
	" formed by its hook \"hook\", if connected."
	" If \"path\" is omitted then \".\" is assumed.",
	{ "disconnect" }
};

static int
RmHookCmd(int ac, char **av)
{
	struct ngm_rmhook rmh;
	const char *path = ".";

	/* Get arguments */
	switch (ac) {
	case 3:
		path = av[1];
		ac--;
		av++;
		/* FALLTHROUGH */
	case 2:
		snprintf(rmh.ourhook, sizeof(rmh.ourhook), "%s", av[1]);
		break;
	default:
		return (CMDRTN_USAGE);
	}

	/* Send message */
	if (NgSendMsg(csock, path, NGM_GENERIC_COOKIE,
	    NGM_RMHOOK, &rmh, sizeof(rmh)) < 0) {
		warn("send msg");
		return (CMDRTN_ERROR);
	}
	return (CMDRTN_OK);
}