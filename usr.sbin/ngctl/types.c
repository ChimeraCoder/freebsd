
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

static int TypesCmd(int ac, char **av);

const struct ngcmd types_cmd = {
	TypesCmd,
	"types",
	"Show information about all installed node types",
	NULL,
	{ NULL }
};

static int
TypesCmd(int ac, char **av __unused)
{
	struct ng_mesg *resp;
	struct typelist *tlist;
	int rtn = CMDRTN_OK;
	u_int k;

	/* Get arguments */
	switch (ac) {
	case 1:
		break;
	default:
		return (CMDRTN_USAGE);
	}

	/* Get list of types */
	if (NgSendMsg(csock, ".", NGM_GENERIC_COOKIE,
	    NGM_LISTTYPES, NULL, 0) < 0) {
		warn("send msg");
		return (CMDRTN_ERROR);
	}
	if (NgAllocRecvMsg(csock, &resp, NULL) < 0) {
		warn("recv msg");
		return (CMDRTN_ERROR);
	}

	/* Show each type */
	tlist = (struct typelist *) resp->data;
	printf("There are %d total types:\n", tlist->numtypes);
	if (tlist->numtypes > 0) {
		printf("%15s   Number of living nodes\n", "Type name");
		printf("%15s   ----------------------\n", "---------");
	}
	for (k = 0; k < tlist->numtypes; k++) {
		struct typeinfo *const ti = &tlist->typeinfo[k];
		printf("%15s   %5d\n", ti->type_name, ti->numnodes);
	}

	/* Done */
	free(resp);
	return (rtn);
}