
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-rrcp.c,v 1.2 2008-04-11 17:21:34 gianluca Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <string.h>

#include "netdissect.h"
#include "addrtoname.h"
#include "extract.h"
#include "ether.h"

#ifndef ETH_ALEN 
#define ETH_ALEN 6
#endif

#define RRCP_OPCODE_MASK	0x7F	/* 0x00 = hello, 0x01 = get, 0x02 = set */
#define RRCP_ISREPLY		0x80	/* 0 = request to switch, 0x80 = reply from switch */

#define RRCP_PROTO_OFFSET		0	/* proto - 1 byte, must be 1 */
#define RRCP_OPCODE_ISREPLY_OFFSET	1	/* opcode and isreply flag - 1 byte */
#define RRCP_AUTHKEY_OFFSET		2	/* authorization key - 2 bytes, 0x2379 by default */

/* most packets */
#define RRCP_REG_ADDR_OFFSET		4	/* register address - 2 bytes */
#define RRCP_REG_DATA_OFFSET		6	/* register data - 4 bytes */
#define RRCP_COOKIE1_OFFSET		10	/* 4 bytes */
#define RRCP_COOKIE2_OFFSET		14	/* 4 bytes */

/* hello reply packets */
#define RRCP_DOWNLINK_PORT_OFFSET	4	/* 1 byte */
#define RRCP_UPLINK_PORT_OFFSET		5	/* 1 byte */
#define RRCP_UPLINK_MAC_OFFSET		6	/* 6 byte MAC address */
#define RRCP_CHIP_ID_OFFSET		12	/* 2 bytes */
#define RRCP_VENDOR_ID_OFFSET		14	/* 4 bytes */

static const struct tok proto_values[] = {
	{ 1, "RRCP" },
	{ 2, "RRCP-REP" },
	{ 0, NULL }
};

static const struct tok opcode_values[] = {
	{ 0, "hello" },
	{ 1, "get" },
	{ 2, "set" },
	{ 0, NULL }
};

/*
 * Print RRCP requests
 */
void
rrcp_print(netdissect_options *ndo,
	  register const u_char *cp,
	  u_int length _U_)
{
	const u_char *rrcp;
	u_int8_t rrcp_proto;
	u_int8_t rrcp_opcode;
	register const struct ether_header *ep;
	char proto_str[16];
	char opcode_str[32];

	ep = (const struct ether_header *)cp;
	rrcp = cp + ETHER_HDRLEN;

	ND_TCHECK(*(rrcp + RRCP_PROTO_OFFSET));
	rrcp_proto = *(rrcp + RRCP_PROTO_OFFSET);
	ND_TCHECK(*(rrcp + RRCP_OPCODE_ISREPLY_OFFSET));
	rrcp_opcode = (*(rrcp + RRCP_OPCODE_ISREPLY_OFFSET)) & RRCP_OPCODE_MASK;
        ND_PRINT((ndo, "%s > %s, %s %s",
		etheraddr_string(ESRC(ep)),
		etheraddr_string(EDST(ep)),
		tok2strbuf(proto_values,"RRCP-0x%02x",rrcp_proto,proto_str,sizeof(proto_str)),
		((*(rrcp + RRCP_OPCODE_ISREPLY_OFFSET)) & RRCP_ISREPLY) ? "reply" : "query"));
	if (rrcp_proto==1){
    	    ND_PRINT((ndo, ": %s",
		     tok2strbuf(opcode_values,"unknown opcode (0x%02x)",rrcp_opcode,opcode_str,sizeof(opcode_str))));
	}
	if (rrcp_opcode==1 || rrcp_opcode==2){
	    ND_TCHECK2(*(rrcp + RRCP_REG_ADDR_OFFSET), 6);
    	    ND_PRINT((ndo, " addr=0x%04x, data=0x%08x",
                     EXTRACT_LE_16BITS(rrcp + RRCP_REG_ADDR_OFFSET),
                     EXTRACT_LE_32BITS(rrcp + RRCP_REG_DATA_OFFSET)));
	}
	if (rrcp_proto==1){
	    ND_TCHECK2(*(rrcp + RRCP_AUTHKEY_OFFSET), 2);
    	    ND_PRINT((ndo, ", auth=0x%04x",
		  EXTRACT_16BITS(rrcp + RRCP_AUTHKEY_OFFSET)));
	}
	if (rrcp_proto==1 && rrcp_opcode==0 &&
	     ((*(rrcp + RRCP_OPCODE_ISREPLY_OFFSET)) & RRCP_ISREPLY)){
	    ND_TCHECK2(*(rrcp + RRCP_VENDOR_ID_OFFSET), 4);
	    ND_PRINT((ndo, " downlink_port=%d, uplink_port=%d, uplink_mac=%s, vendor_id=%08x ,chip_id=%04x ",
		     *(rrcp + RRCP_DOWNLINK_PORT_OFFSET),
		     *(rrcp + RRCP_UPLINK_PORT_OFFSET),
		     etheraddr_string(rrcp + RRCP_UPLINK_MAC_OFFSET),
		     EXTRACT_32BITS(rrcp + RRCP_VENDOR_ID_OFFSET),
		     EXTRACT_16BITS(rrcp + RRCP_CHIP_ID_OFFSET)));
	}else if (rrcp_opcode==1 || rrcp_opcode==2 || rrcp_proto==2){
	    ND_TCHECK2(*(rrcp + RRCP_COOKIE2_OFFSET), 4);
	    ND_PRINT((ndo, ", cookie=0x%08x%08x ",
		    EXTRACT_32BITS(rrcp + RRCP_COOKIE2_OFFSET),
		    EXTRACT_32BITS(rrcp + RRCP_COOKIE1_OFFSET)));
	}
        if (!ndo->ndo_vflag)
            return;
	return;

trunc:
	ND_PRINT((ndo, "[|rrcp]"));
}