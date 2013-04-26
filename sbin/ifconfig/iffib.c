
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
#include <sys/socket.h>
#include <sys/sockio.h>

#include <net/if.h>
#include <net/route.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "ifconfig.h"

static void
fib_status(int s)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

	if (ioctl(s, SIOCGIFFIB, (caddr_t)&ifr) < 0)
		return;

	/* Ignore if it is the default. */
	if (ifr.ifr_fib == 0)
		return;

	printf("\tfib: %u\n", ifr.ifr_fib);
}

static void
setiffib(const char *val, int dummy __unused, int s,
    const struct afswtch *afp)
{
	unsigned long fib;
	char *ep;

	fib = strtoul(val, &ep, 0);
	if (*ep != '\0' || fib > UINT_MAX) {
		warn("fib %s not valid", val);
		return;
	}

	strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
	ifr.ifr_fib = fib;
	if (ioctl(s, SIOCSIFFIB, (caddr_t)&ifr) < 0)
		warn("ioctl (SIOCSIFFIB)");
}

static struct cmd fib_cmds[] = {
	DEF_CMD_ARG("fib", setiffib),
};

static struct afswtch af_fib = {
	.af_name	= "af_fib",
	.af_af		= AF_UNSPEC,
	.af_other_status = fib_status,
};

static __constructor void
fib_ctor(void)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	size_t i;

	for (i = 0; i < N(fib_cmds);  i++)
		cmd_register(&fib_cmds[i]);
	af_register(&af_fib);
#undef N
}