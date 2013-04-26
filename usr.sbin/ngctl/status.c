
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
#include <errno.h>
#include <netgraph.h>
#include <stdio.h>

#include "ngctl.h"

#define NOSTATUS	"<no status>"

static int StatusCmd(int ac, char **av);

const struct ngcmd status_cmd = {
	StatusCmd,
	"status <path>",
	"Get human readable status information from the node at <path>",
	NULL,
	{ NULL }
};

static int
StatusCmd(int ac, char **av)
{
	u_char sbuf[sizeof(struct ng_mesg) + NG_TEXTRESPONSE];
	struct ng_mesg *const resp = (struct ng_mesg *) sbuf;
	char *const status = (char *) resp->data;
	char *path;
	int nostat = 0;

	/* Get arguments */
	switch (ac) {
	case 2:
		path = av[1];
		break;
	default:
		return (CMDRTN_USAGE);
	}

	/* Get node status summary */
	if (NgSendMsg(csock, path, NGM_GENERIC_COOKIE,
	    NGM_TEXT_STATUS, NULL, 0) < 0) {
		switch (errno) {
		case EINVAL:
			nostat = 1;
			break;
		default:
			warn("send msg");
			return (CMDRTN_ERROR);
		}
	} else {
		if (NgRecvMsg(csock, resp, sizeof(sbuf), NULL) < 0
		    || (resp->header.flags & NGF_RESP) == 0)
			nostat = 1;
	}

	/* Show it */
	if (nostat)
		printf("No status available for \"%s\"\n", path);
	else
		printf("Status for \"%s\":\n%s\n", path, status);
	return (CMDRTN_OK);
}