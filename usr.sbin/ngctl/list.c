
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
#include <stdlib.h>
#include <unistd.h>

#include "ngctl.h"

#define UNNAMED		"<unnamed>"

static int ListCmd(int ac, char **av);

const struct ngcmd list_cmd = {
	ListCmd,
	"list [-ln]",
	"Show information about all nodes",
	"The list command shows information about every node that currently"
	" exists in the netgraph system. The optional -n argument limits"
	" this list to only those nodes with a global name assignment."
	" The optional -l argument provides verbose output that includes"
	" hook information as well.",
	{ "ls" }
};

static int
ListCmd(int ac, char **av)
{
	struct ng_mesg *resp;
	struct namelist *nlist;
	struct nodeinfo *ninfo;
	int list_hooks = 0;
	int named_only = 0;
	int ch, rtn = CMDRTN_OK;

	/* Get options */
	optind = 1;
	while ((ch = getopt(ac, av, "ln")) != -1) {
		switch (ch) {
		case 'l':
			list_hooks = 1;
			break;
		case 'n':
			named_only = 1;
			break;
		case '?':
		default:
			return (CMDRTN_USAGE);
			break;
		}
	}
	ac -= optind;
	av += optind;

	/* Get arguments */
	switch (ac) {
	case 0:
		break;
	default:
		return (CMDRTN_USAGE);
	}

	/* Get list of nodes */
	if (NgSendMsg(csock, ".", NGM_GENERIC_COOKIE,
	    named_only ? NGM_LISTNAMES : NGM_LISTNODES, NULL, 0) < 0) {
		warn("send msg");
		return (CMDRTN_ERROR);
	}
	if (NgAllocRecvMsg(csock, &resp, NULL) < 0) {
		warn("recv msg");
		return (CMDRTN_ERROR);
	}

	/* Show each node */
	nlist = (struct namelist *) resp->data;
	printf("There are %d total %snodes:\n",
	    nlist->numnames, named_only ? "named " : "");
	ninfo = nlist->nodeinfo;
	if (list_hooks) {
		char	path[NG_PATHSIZ];
		char	*argv[2] = { "show", path };

		while (nlist->numnames > 0) {
			snprintf(path, sizeof(path),
			    "[%lx]:", (u_long)ninfo->id);
			if ((rtn = (*show_cmd.func)(2, argv)) != CMDRTN_OK)
				break;
			ninfo++;
			nlist->numnames--;
		}
	} else {
		while (nlist->numnames > 0) {
			if (!*ninfo->name)
				snprintf(ninfo->name, sizeof(ninfo->name),
				    "%s", UNNAMED);
			printf("  Name: %-15s Type: %-15s ID: %08x   "
			    "Num hooks: %d\n",
			    ninfo->name, ninfo->type, ninfo->id, ninfo->hooks);
			ninfo++;
			nlist->numnames--;
		}
	}

	/* Done */
	free(resp);
	return (rtn);
}