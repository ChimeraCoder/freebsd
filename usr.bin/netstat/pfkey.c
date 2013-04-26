
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)inet.c	8.5 (Berkeley) 5/24/95";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <netinet/in.h>

#ifdef IPSEC
#include <netipsec/keysock.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "netstat.h"

#ifdef IPSEC

static const char *pfkey_msgtypenames[] = {
	"reserved", "getspi", "update", "add", "delete",
	"get", "acquire", "register", "expire", "flush",
	"dump", "x_promisc", "x_pchange", "x_spdupdate", "x_spdadd",
	"x_spddelete", "x_spdget", "x_spdacquire", "x_spddump", "x_spdflush",
	"x_spdsetidx", "x_spdexpire", "x_spddelete2"
};

static const char *pfkey_msgtype_names (int);


static const char *
pfkey_msgtype_names(int x)
{
	const int max =
	    sizeof(pfkey_msgtypenames)/sizeof(pfkey_msgtypenames[0]);
	static char buf[20];

	if (x < max && pfkey_msgtypenames[x])
		return pfkey_msgtypenames[x];
	snprintf(buf, sizeof(buf), "#%d", x);
	return buf;
}

void
pfkey_stats(u_long off, const char *name, int family __unused,
    int proto __unused)
{
	struct pfkeystat pfkeystat;
	unsigned first, type;

	if (off == 0)
		return;
	printf ("%s:\n", name);
	kread(off, (char *)&pfkeystat, sizeof(pfkeystat));

#define	p(f, m) if (pfkeystat.f || sflag <= 1) \
    printf(m, (uintmax_t)pfkeystat.f, plural(pfkeystat.f))

	/* userland -> kernel */
	p(out_total, "\t%ju request%s sent from userland\n");
	p(out_bytes, "\t%ju byte%s sent from userland\n");
	for (first = 1, type = 0;
	     type < sizeof(pfkeystat.out_msgtype)/sizeof(pfkeystat.out_msgtype[0]);
	     type++) {
		if (pfkeystat.out_msgtype[type] <= 0)
			continue;
		if (first) {
			printf("\thistogram by message type:\n");
			first = 0;
		}
		printf("\t\t%s: %ju\n", pfkey_msgtype_names(type),
			(uintmax_t)pfkeystat.out_msgtype[type]);
	}
	p(out_invlen, "\t%ju message%s with invalid length field\n");
	p(out_invver, "\t%ju message%s with invalid version field\n");
	p(out_invmsgtype, "\t%ju message%s with invalid message type field\n");
	p(out_tooshort, "\t%ju message%s too short\n");
	p(out_nomem, "\t%ju message%s with memory allocation failure\n");
	p(out_dupext, "\t%ju message%s with duplicate extension\n");
	p(out_invexttype, "\t%ju message%s with invalid extension type\n");
	p(out_invsatype, "\t%ju message%s with invalid sa type\n");
	p(out_invaddr, "\t%ju message%s with invalid address extension\n");

	/* kernel -> userland */
	p(in_total, "\t%ju request%s sent to userland\n");
	p(in_bytes, "\t%ju byte%s sent to userland\n");
	for (first = 1, type = 0;
	     type < sizeof(pfkeystat.in_msgtype)/sizeof(pfkeystat.in_msgtype[0]);
	     type++) {
		if (pfkeystat.in_msgtype[type] <= 0)
			continue;
		if (first) {
			printf("\thistogram by message type:\n");
			first = 0;
		}
		printf("\t\t%s: %ju\n", pfkey_msgtype_names(type),
			(uintmax_t)pfkeystat.in_msgtype[type]);
	}
	p(in_msgtarget[KEY_SENDUP_ONE],
	    "\t%ju message%s toward single socket\n");
	p(in_msgtarget[KEY_SENDUP_ALL],
	    "\t%ju message%s toward all sockets\n");
	p(in_msgtarget[KEY_SENDUP_REGISTERED],
	    "\t%ju message%s toward registered sockets\n");
	p(in_nomem, "\t%ju message%s with memory allocation failure\n");
#undef p
}
#endif /* IPSEC */