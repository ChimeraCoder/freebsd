
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
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/tcpdump/print-vrrp.c,v 1.10 2005-05-06 07:56:54 guy Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <stdlib.h>

#include "interface.h"
#include "extract.h"
#include "addrtoname.h"

/*
 * RFC 2338:
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |Version| Type  | Virtual Rtr ID|   Priority    | Count IP Addrs|
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |   Auth Type   |   Adver Int   |          Checksum             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                         IP Address (1)                        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                            .                                  |
 *    |                            .                                  |
 *    |                            .                                  |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                         IP Address (n)                        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                     Authentication Data (1)                   |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                     Authentication Data (2)                   |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/* Type */
#define	VRRP_TYPE_ADVERTISEMENT	1

static const struct tok type2str[] = {
	{ VRRP_TYPE_ADVERTISEMENT,	"Advertisement"	},
	{ 0,				NULL		}
};

/* Auth Type */
#define	VRRP_AUTH_NONE		0
#define	VRRP_AUTH_SIMPLE	1
#define	VRRP_AUTH_AH		2

static const struct tok auth2str[] = {
	{ VRRP_AUTH_NONE,		"none"		},
	{ VRRP_AUTH_SIMPLE,		"simple"	},
	{ VRRP_AUTH_AH,			"ah"		},
	{ 0,				NULL		}
};

void
vrrp_print(register const u_char *bp, register u_int len, int ttl)
{
	int version, type, auth_type;
	const char *type_s;

	TCHECK(bp[0]);
	version = (bp[0] & 0xf0) >> 4;
	type = bp[0] & 0x0f;
	type_s = tok2str(type2str, "unknown type (%u)", type);
	printf("VRRPv%u, %s", version, type_s);
	if (ttl != 255)
		printf(", (ttl %u)", ttl);
	if (version != 2 || type != VRRP_TYPE_ADVERTISEMENT)
		return;
	TCHECK(bp[2]);
	printf(", vrid %u, prio %u", bp[1], bp[2]);
	TCHECK(bp[5]);
	auth_type = bp[4];
	printf(", authtype %s", tok2str(auth2str, NULL, auth_type));
	printf(", intvl %us, length %u", bp[5],len);
	if (vflag) {
		int naddrs = bp[3];
		int i;
		char c;

		if (TTEST2(bp[0], len)) {
			struct cksum_vec vec[1];

			vec[0].ptr = bp;
			vec[0].len = len;
			if (in_cksum(vec, 1))
				printf(", (bad vrrp cksum %x)",
					EXTRACT_16BITS(&bp[6]));
		}
		printf(", addrs");
		if (naddrs > 1)
			printf("(%d)", naddrs);
		printf(":");
		c = ' ';
		bp += 8;
		for (i = 0; i < naddrs; i++) {
			TCHECK(bp[3]);
			printf("%c%s", c, ipaddr_string(bp));
			c = ',';
			bp += 4;
		}
		if (auth_type == VRRP_AUTH_SIMPLE) { /* simple text password */
			TCHECK(bp[7]);
			printf(" auth \"");
			if (fn_printn(bp, 8, snapend)) {
				printf("\"");
				goto trunc;
			}
			printf("\"");
		}
	}
	return;
trunc:
	printf("[|vrrp]");
}