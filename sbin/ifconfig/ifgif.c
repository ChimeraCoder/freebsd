
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
#include <net/if_gif.h>
#include <net/route.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

#include "ifconfig.h"

#define	GIFBITS	"\020\1ACCEPT_REV_ETHIP_VER\5SEND_REV_ETHIP_VER"

static void	gif_status(int);

static void
gif_status(int s)
{
	int opts;

	ifr.ifr_data = (caddr_t)&opts;
	if (ioctl(s, GIFGOPTS, &ifr) == -1)
		return;
	if (opts == 0)
		return;
	printb("\toptions", opts, GIFBITS);
	putchar('\n');
}

static void
setgifopts(const char *val,
	int d, int s, const struct afswtch *afp)
{
	int opts;

	ifr.ifr_data = (caddr_t)&opts;
	if (ioctl(s, GIFGOPTS, &ifr) == -1) {
		warn("ioctl(GIFGOPTS)");
		return;
	}

	if (d < 0)
		opts &= ~(-d);
	else
		opts |= d;

	if (ioctl(s, GIFSOPTS, &ifr) == -1) {
		warn("ioctl(GIFSOPTS)");
		return;
	}
}

static struct cmd gif_cmds[] = {
	DEF_CMD("accept_rev_ethip_ver",	GIF_ACCEPT_REVETHIP,	setgifopts),
	DEF_CMD("-accept_rev_ethip_ver",-GIF_ACCEPT_REVETHIP,	setgifopts),
	DEF_CMD("send_rev_ethip_ver",	GIF_SEND_REVETHIP,	setgifopts),
	DEF_CMD("-send_rev_ethip_ver",	-GIF_SEND_REVETHIP,	setgifopts),
};

static struct afswtch af_gif = {
	.af_name	= "af_gif",
	.af_af		= AF_UNSPEC,
	.af_other_status = gif_status,
};

static __constructor void
gif_ctor(void)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	size_t i;

	for (i = 0; i < N(gif_cmds); i++)
		cmd_register(&gif_cmds[i]);
	af_register(&af_gif);
#undef N
}