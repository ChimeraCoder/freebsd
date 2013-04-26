
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

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/mac.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#include <net/if.h>
#include <net/route.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ifconfig.h"

static void
maclabel_status(int s)
{
	struct ifreq ifr;
	mac_t label;
	char *label_text;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

	if (mac_prepare_ifnet_label(&label) == -1)
		return;
	ifr.ifr_ifru.ifru_data = (void *)label;
	if (ioctl(s, SIOCGIFMAC, &ifr) == -1)
		goto mac_free;

	
	if (mac_to_text(label, &label_text) == -1)
		goto mac_free;

	if (strlen(label_text) != 0)
		printf("\tmaclabel %s\n", label_text);
	free(label_text);

mac_free:
	mac_free(label);
}

static void
setifmaclabel(const char *val, int d, int s, const struct afswtch *rafp)
{
	struct ifreq ifr;
	mac_t label;
	int error;

	if (mac_from_text(&label, val) == -1) {
		perror(val);
		return;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	ifr.ifr_ifru.ifru_data = (void *)label;

	error = ioctl(s, SIOCSIFMAC, &ifr);
	mac_free(label);
	if (error == -1)
		perror("setifmac");
}

static struct cmd mac_cmds[] = {
	DEF_CMD_ARG("maclabel",	setifmaclabel),
};
static struct afswtch af_mac = {
	.af_name	= "af_maclabel",
	.af_af		= AF_UNSPEC,
	.af_other_status = maclabel_status,
};

static __constructor void
mac_ctor(void)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	size_t i;

	for (i = 0; i < N(mac_cmds);  i++)
		cmd_register(&mac_cmds[i]);
	af_register(&af_mac);
#undef N
}