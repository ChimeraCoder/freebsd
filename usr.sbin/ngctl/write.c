
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

#include <sys/types.h>
#include <sys/socket.h>

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netgraph/ng_socket.h>

#include "ngctl.h"

#define BUF_SIZE	8192

static int WriteCmd(int ac, char **av);

const struct ngcmd write_cmd = {
	WriteCmd,
	"write hook < -f file | byte ... >",
	"Send a data packet down the hook named by \"hook\".",
	"The data may be contained in a file, or may be described directly"
	" on the command line by supplying a sequence of bytes.",
	{ "w" }
};

static int
WriteCmd(int ac, char **av)
{
	u_int32_t sagbuf[64];
	struct sockaddr_ng *sag = (struct sockaddr_ng *)sagbuf;
	u_char buf[BUF_SIZE];
	const char *hook;
	FILE *fp;
	u_int len;
	int byte;
	int i;

	/* Get arguments */
	if (ac < 3)
		return (CMDRTN_USAGE);
	hook = av[1];

	/* Get data */
	if (strcmp(av[2], "-f") == 0) {
		if (ac != 4)
			return (CMDRTN_USAGE);
		if ((fp = fopen(av[3], "r")) == NULL) {
			warn("can't read file \"%s\"", av[3]);
			return (CMDRTN_ERROR);
		}
		if ((len = fread(buf, 1, sizeof(buf), fp)) == 0) {
			if (ferror(fp))
				warn("can't read file \"%s\"", av[3]);
			else
				warnx("file \"%s\" is empty", av[3]);
			fclose(fp);
			return (CMDRTN_ERROR);
		}
		fclose(fp);
	} else {
		for (i = 2, len = 0; i < ac && len < sizeof(buf); i++, len++) {
			if (sscanf(av[i], "%i", &byte) != 1
			    || (byte < -128 || byte > 255)) {
				warnx("invalid byte \"%s\"", av[i]);
				return (CMDRTN_ERROR);
			}
			buf[len] = (u_char)byte;
		}
		if (len == 0)
			return (CMDRTN_USAGE);
	}

	/* Send data */
	sag->sg_len = 3 + strlen(hook);
	sag->sg_family = AF_NETGRAPH;
	strlcpy(sag->sg_data, hook, sizeof(sagbuf) - 2);
	if (sendto(dsock, buf, len,
	    0, (struct sockaddr *)sag, sag->sg_len) == -1) {
		warn("writing to hook \"%s\"", hook);
		return (CMDRTN_ERROR);
	}

	/* Done */
	return (CMDRTN_OK);
}