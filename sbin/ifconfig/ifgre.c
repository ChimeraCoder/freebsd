
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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#include <stdlib.h>
#include <unistd.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_gre.h>
#include <net/route.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

#include "ifconfig.h"

static	void gre_status(int s);

static void
gre_status(int s)
{
	int grekey = 0;

	ifr.ifr_data = (caddr_t)&grekey;
	if (ioctl(s, GREGKEY, &ifr) == 0)
		if (grekey != 0)
			printf("\tgrekey: %d\n", grekey);
}

static void
setifgrekey(const char *val, int dummy __unused, int s, 
    const struct afswtch *afp)
{
	uint32_t grekey = atol(val);

	strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
	ifr.ifr_data = (caddr_t)&grekey;
	if (ioctl(s, GRESKEY, (caddr_t)&ifr) < 0)
		warn("ioctl (set grekey)");
}

static struct cmd gre_cmds[] = {
	DEF_CMD_ARG("grekey",			setifgrekey),
};
static struct afswtch af_gre = {
	.af_name	= "af_gre",
	.af_af		= AF_UNSPEC,
	.af_other_status = gre_status,
};

static __constructor void
gre_ctor(void)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	size_t i;

	for (i = 0; i < N(gre_cmds);  i++)
		cmd_register(&gre_cmds[i]);
	af_register(&af_gre);
#undef N
}