
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
#include <string.h>
#include <unistd.h>

#include "ngctl.h"

#define NOCONFIG	"<no config>"

static int ConfigCmd(int ac, char **av);

const struct ngcmd config_cmd = {
	ConfigCmd,
	"config <path> [arguments]",
	"get or set configuration of node at <path>",
	NULL,
	{ NULL }
};

static int
ConfigCmd(int ac, char **av)
{
	u_char sbuf[sizeof(struct ng_mesg) + NG_TEXTRESPONSE];
	struct ng_mesg *const resp = (struct ng_mesg *) sbuf;
	char *const status = (char *) resp->data;
	char *path;
	char buf[NG_TEXTRESPONSE];
	int nostat = 0, i;

	/* Get arguments */
	if (ac < 2)
		return (CMDRTN_USAGE);
	path = av[1];

	*buf = '\0';
	for (i = 2; i < ac; i++) {
		if (i != 2)
			strcat(buf, " ");
		strcat(buf, av[i]);
	}
	
	/* Get node config summary */
	if (*buf != '\0')
		i = NgSendMsg(csock, path, NGM_GENERIC_COOKIE,
	            NGM_TEXT_CONFIG, buf, strlen(buf) + 1);
	else
		i = NgSendMsg(csock, path, NGM_GENERIC_COOKIE,
	            NGM_TEXT_CONFIG, NULL, 0);
	if (i < 0) {
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
		printf("No config available for \"%s\"\n", path);
	else
		printf("Config for \"%s\":\n%s\n", path, status);
	return (CMDRTN_OK);
}