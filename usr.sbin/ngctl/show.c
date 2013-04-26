
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

#define FMT		"  %-15s %-15s %-12s %-15s %-15s\n"
#define UNNAMED		"<unnamed>"
#define NOSTATUS	"<no status>"

static int ShowCmd(int ac, char **av);

const struct ngcmd show_cmd = {
	ShowCmd,
	"show [-n] <path>",
	"Show information about the node at <path>",
	"If the -n flag is given, hooks are not listed.",
	{ "inquire", "info" }
};

static int
ShowCmd(int ac, char **av)
{
	char *path;
	struct ng_mesg *resp;
	struct hooklist *hlist;
	struct nodeinfo *ninfo;
	int ch, no_hooks = 0;

	/* Get options */
	optind = 1;
	while ((ch = getopt(ac, av, "n")) != -1) {
		switch (ch) {
		case 'n':
			no_hooks = 1;
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
	case 1:
		path = av[0];
		break;
	default:
		return (CMDRTN_USAGE);
	}

	/* Get node info and hook list */
	if (NgSendMsg(csock, path, NGM_GENERIC_COOKIE,
	    NGM_LISTHOOKS, NULL, 0) < 0) {
		warn("send msg");
		return (CMDRTN_ERROR);
	}
	if (NgAllocRecvMsg(csock, &resp, NULL) < 0) {
		warn("recv msg");
		return (CMDRTN_ERROR);
	}

	/* Show node information */
	hlist = (struct hooklist *) resp->data;
	ninfo = &hlist->nodeinfo;
	if (!*ninfo->name)
		snprintf(ninfo->name, sizeof(ninfo->name), "%s", UNNAMED);
	printf("  Name: %-15s Type: %-15s ID: %08x   Num hooks: %d\n",
	    ninfo->name, ninfo->type, ninfo->id, ninfo->hooks);
	if (!no_hooks && ninfo->hooks > 0) {
		u_int k;

		printf(FMT, "Local hook", "Peer name",
		    "Peer type", "Peer ID", "Peer hook");
		printf(FMT, "----------", "---------",
		    "---------", "-------", "---------");
		for (k = 0; k < ninfo->hooks; k++) {
			struct linkinfo *const link = &hlist->link[k];
			struct nodeinfo *const peer = &hlist->link[k].nodeinfo;
			char idbuf[20];

			if (!*peer->name) {
				snprintf(peer->name, sizeof(peer->name),
				  "%s", UNNAMED);
			}
			snprintf(idbuf, sizeof(idbuf), "%08x", peer->id);
			printf(FMT, link->ourhook, peer->name,
			    peer->type, idbuf, link->peerhook);
		}
	}
	free(resp);
	return (CMDRTN_OK);
}