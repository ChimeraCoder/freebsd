
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
"@(#) $Header: /tcpdump/master/tcpdump/print-pppoe.c,v 1.31 2005-04-26 19:48:38 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <string.h>

#include "interface.h"
#include "addrtoname.h"
#include "ppp.h"
#include "ethertype.h"
#include "ether.h"
#include "extract.h"			/* must come after interface.h */

/* Codes */
enum {
	PPPOE_PADI = 0x09,
	PPPOE_PADO = 0x07,
	PPPOE_PADR = 0x19,
	PPPOE_PADS = 0x65,
	PPPOE_PADT = 0xa7
};

static struct tok pppoecode2str[] = {
	{ PPPOE_PADI, "PADI" },
	{ PPPOE_PADO, "PADO" },
	{ PPPOE_PADR, "PADR" },
	{ PPPOE_PADS, "PADS" },
	{ PPPOE_PADT, "PADT" },
	{ 0, "" }, /* PPP Data */
	{ 0, NULL }
};

/* Tags */
enum {
	PPPOE_EOL = 0,
	PPPOE_SERVICE_NAME = 0x0101,
	PPPOE_AC_NAME = 0x0102,
	PPPOE_HOST_UNIQ = 0x0103,
	PPPOE_AC_COOKIE = 0x0104,
	PPPOE_VENDOR = 0x0105,
	PPPOE_RELAY_SID = 0x0110,
	PPPOE_MAX_PAYLOAD = 0x0120,
	PPPOE_SERVICE_NAME_ERROR = 0x0201,
	PPPOE_AC_SYSTEM_ERROR = 0x0202,
	PPPOE_GENERIC_ERROR = 0x0203
};

static struct tok pppoetag2str[] = {
	{ PPPOE_EOL, "EOL" },
	{ PPPOE_SERVICE_NAME, "Service-Name" },
	{ PPPOE_AC_NAME, "AC-Name" },
	{ PPPOE_HOST_UNIQ, "Host-Uniq" },
	{ PPPOE_AC_COOKIE, "AC-Cookie" },
	{ PPPOE_VENDOR, "Vendor-Specific" },
	{ PPPOE_RELAY_SID, "Relay-Session-ID" },
	{ PPPOE_MAX_PAYLOAD, "PPP-Max-Payload" },
	{ PPPOE_SERVICE_NAME_ERROR, "Service-Name-Error" },
	{ PPPOE_AC_SYSTEM_ERROR, "AC-System-Error" },
	{ PPPOE_GENERIC_ERROR, "Generic-Error" },
	{ 0, NULL }
};

#define PPPOE_HDRLEN 6
#define MAXTAGPRINT 80

u_int
pppoe_if_print(const struct pcap_pkthdr *h, register const u_char *p)
{
	return (pppoe_print(p, h->len));
}

u_int
pppoe_print(register const u_char *bp, u_int length)
{
	u_int16_t pppoe_ver, pppoe_type, pppoe_code, pppoe_sessionid;
	u_int pppoe_length;
	const u_char *pppoe_packet, *pppoe_payload;

	if (length < PPPOE_HDRLEN) {
		(void)printf("truncated-pppoe %u", length);
		return (length);
	}
	length -= PPPOE_HDRLEN;
	pppoe_packet = bp;
	TCHECK2(*pppoe_packet, PPPOE_HDRLEN);
	pppoe_ver  = (pppoe_packet[0] & 0xF0) >> 4;
	pppoe_type  = (pppoe_packet[0] & 0x0F);
	pppoe_code = pppoe_packet[1];
	pppoe_sessionid = EXTRACT_16BITS(pppoe_packet + 2);
	pppoe_length    = EXTRACT_16BITS(pppoe_packet + 4);
	pppoe_payload = pppoe_packet + PPPOE_HDRLEN;

	if (pppoe_ver != 1) {
		printf(" [ver %d]",pppoe_ver);
	}
	if (pppoe_type != 1) {
		printf(" [type %d]",pppoe_type);
	}

	printf("PPPoE %s", tok2str(pppoecode2str, "PAD-%x", pppoe_code));
	if (pppoe_code == PPPOE_PADI && pppoe_length > 1484 - PPPOE_HDRLEN) {
		printf(" [len %u!]",pppoe_length);
	}
	if (pppoe_length > length) {
		printf(" [len %u > %u!]", pppoe_length, length);
		pppoe_length = length;
	}
	if (pppoe_sessionid) {
		printf(" [ses 0x%x]", pppoe_sessionid);
	}

	if (pppoe_code) {
		/* PPP session packets don't contain tags */
		u_short tag_type = 0xffff, tag_len;
		const u_char *p = pppoe_payload;

		/*
		 * loop invariant:
		 * p points to current tag,
		 * tag_type is previous tag or 0xffff for first iteration
		 */
		while (tag_type && p < pppoe_payload + pppoe_length) {
			TCHECK2(*p, 4);
			tag_type = EXTRACT_16BITS(p);
			tag_len = EXTRACT_16BITS(p + 2);
			p += 4;
			/* p points to tag_value */

			if (tag_len) {
				unsigned isascii = 0, isgarbage = 0;
				const u_char *v = p;
				char tag_str[MAXTAGPRINT];
				unsigned tag_str_len = 0;

				/* TODO print UTF-8 decoded text */
				TCHECK2(*p, tag_len);
				for (v = p; v < p + tag_len && tag_str_len < MAXTAGPRINT-1; v++)
					if (*v >= 32 && *v < 127) {
						tag_str[tag_str_len++] = *v;
						isascii++;
					} else {
						tag_str[tag_str_len++] = '.';
						isgarbage++;
					}
				tag_str[tag_str_len] = 0;

				if (isascii > isgarbage) {
					printf(" [%s \"%*.*s\"]",
					       tok2str(pppoetag2str, "TAG-0x%x", tag_type),
					       (int)tag_str_len,
					       (int)tag_str_len,
					       tag_str);
				} else {
					/* Print hex, not fast to abuse printf but this doesn't get used much */
					printf(" [%s 0x", tok2str(pppoetag2str, "TAG-0x%x", tag_type));
					for (v=p; v<p+tag_len; v++) {
						printf("%02X", *v);
					}
					printf("]");
				}
				

			} else
				printf(" [%s]", tok2str(pppoetag2str,
				    "TAG-0x%x", tag_type));

			p += tag_len;
			/* p points to next tag */
		}
		return (0);
	} else {
		/* PPPoE data */
		printf(" ");
		return (PPPOE_HDRLEN + ppp_print(pppoe_payload, pppoe_length));
	}

trunc:
	printf("[|pppoe]");
	return (PPPOE_HDRLEN);
}