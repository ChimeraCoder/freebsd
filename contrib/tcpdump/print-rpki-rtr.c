
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
"@(#) $Header: /tcpdump/master/tcpdump/print-rpki_rtr.c,v 1.10 2008-03-20 09:30:56 hannes Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface.h"
#include "extract.h"
#include "addrtoname.h"

/*
 * RPKI/Router PDU header
 *
 * Here's what the PDU header looks like.
 * The length does include the version and length fields.
 */
typedef struct rpki_rtr_pdu_ {
    u_char version;		/* Version number */
    u_char pdu_type;		/* PDU type */
    union {
	u_char cache_nonce[2];	/* Cache Nonce */
	u_char error_code[2];	/* Error code */
    } u;
    u_char length[4];
} rpki_rtr_pdu;
#define RPKI_RTR_PDU_OVERHEAD (offsetof(rpki_rtr_pdu, rpki_rtr_pdu_msg))

/*
 * IPv4 Prefix PDU.
 */
typedef struct rpki_rtr_pdu_ipv4_prefix_ {
    rpki_rtr_pdu pdu_header;
    u_char flags;
    u_char prefix_length;
    u_char max_length;
    u_char zero;
    u_char prefix[4];
    u_char as[4];
} rpki_rtr_pdu_ipv4_prefix;

/*
 * IPv6 Prefix PDU.
 */
typedef struct rpki_rtr_pdu_ipv6_prefix_ {
    rpki_rtr_pdu pdu_header;
    u_char flags;
    u_char prefix_length;
    u_char max_length;
    u_char zero;
    u_char prefix[16];
    u_char as[4];
} rpki_rtr_pdu_ipv6_prefix;

/*
 * Error report PDU.
 */
typedef struct rpki_rtr_pdu_error_report_ {
    rpki_rtr_pdu pdu_header;
    u_char encapsulated_pdu_length[4]; /* Encapsulated PDU length */
} rpki_rtr_pdu_error_report;

/*
 * PDU type codes
 */
#define RPKI_RTR_SERIAL_NOTIFY_PDU	0
#define RPKI_RTR_SERIAL_QUERY_PDU	1
#define RPKI_RTR_RESET_QUERY_PDU	2
#define RPKI_RTR_CACHE_RESPONSE_PDU	3
#define RPKI_RTR_IPV4_PREFIX_PDU	4
#define RPKI_RTR_IPV6_PREFIX_PDU	6
#define RPKI_RTR_END_OF_DATA_PDU	7
#define RPKI_RTR_CACHE_RESET_PDU	8
#define RPKI_RTR_ERROR_REPORT_PDU	10

static const struct tok rpki_rtr_pdu_values[] = {
    { RPKI_RTR_SERIAL_NOTIFY_PDU, "Serial Notify" },
    { RPKI_RTR_SERIAL_QUERY_PDU, "Serial Query" },
    { RPKI_RTR_RESET_QUERY_PDU, "Reset Query" },
    { RPKI_RTR_CACHE_RESPONSE_PDU, "Cache Response" },
    { RPKI_RTR_IPV4_PREFIX_PDU, "IPV4 Prefix" },
    { RPKI_RTR_IPV6_PREFIX_PDU, "IPV6 Prefix" },
    { RPKI_RTR_END_OF_DATA_PDU, "End of Data" },
    { RPKI_RTR_CACHE_RESET_PDU, "Cache Reset" },
    { RPKI_RTR_ERROR_REPORT_PDU, "Error Report" },
    { 0, NULL}
};

static const struct tok rpki_rtr_error_codes[] = {
    { 0, "Corrupt Data" },
    { 1, "Internal Error" },
    { 2, "No Data Available" },
    { 3, "Invalid Request" },
    { 4, "Unsupported Protocol Version" },
    { 5, "Unsupported PDU Type" },
    { 6, "Withdrawal of Unknown Record" },
    { 7, "Duplicate Announcement Received" },
    { 0, NULL}
};

/*
 * Build a identation string for a given identation level.
 * XXX this should be really in util.c
 */
static char *
indent_string (u_int indent)
{
    static char buf[20];
    u_int idx;

    idx = 0;
    buf[idx] = '\0';

    /*
     * Does the static buffer fit ?
     */
    if (sizeof(buf) < ((indent/8) + (indent %8) + 2)) {
	return buf;
    }

    /*
     * Heading newline.
     */
    buf[idx] = '\n';
    idx++;

    while (indent >= 8) {
	buf[idx] = '\t';
	idx++;
	indent -= 8;
    }

    while (indent > 0) {
	buf[idx] = ' ';
	idx++;
	indent--;
    }

    /*
     * Trailing zero.
     */
    buf[idx] = '\0';
    
    return buf;
}

/*
 * Print a single PDU.
 */
static void
rpki_rtr_pdu_print (const u_char *tptr, u_int indent)
{
    const rpki_rtr_pdu *pdu_header;
    u_int pdu_type, pdu_len, hexdump;
    const u_char *msg;

    pdu_header = (rpki_rtr_pdu *)tptr;
    pdu_type = pdu_header->pdu_type;
    pdu_len = EXTRACT_32BITS(pdu_header->length);
    hexdump = FALSE;

    printf("%sRPKI-RTRv%u, %s PDU (%u), length: %u",
	   indent_string(8),
	   pdu_header->version,
	   tok2str(rpki_rtr_pdu_values, "Unknown", pdu_type),
	   pdu_type, pdu_len);

    switch (pdu_type) {

	/*
	 * The following PDUs share the message format.
	 */
    case RPKI_RTR_SERIAL_NOTIFY_PDU:
    case RPKI_RTR_SERIAL_QUERY_PDU:
    case RPKI_RTR_END_OF_DATA_PDU:
        msg = (const u_char *)(pdu_header + 1);
	printf("%sCache-Nonce: 0x%04x, Serial: %u",
	       indent_string(indent+2),
	       EXTRACT_16BITS(pdu_header->u.cache_nonce),
	       EXTRACT_32BITS(msg));
	break;

	/*
	 * The following PDUs share the message format.
	 */
    case RPKI_RTR_RESET_QUERY_PDU:
    case RPKI_RTR_CACHE_RESET_PDU:

	/*
	 * Zero payload PDUs.
	 */
	break;

    case RPKI_RTR_CACHE_RESPONSE_PDU:
	printf("%sCache-Nonce: 0x%04x",
	       indent_string(indent+2),
	       EXTRACT_16BITS(pdu_header->u.cache_nonce));
	break;

    case RPKI_RTR_IPV4_PREFIX_PDU:
	{
	    rpki_rtr_pdu_ipv4_prefix *pdu;

	    pdu = (rpki_rtr_pdu_ipv4_prefix *)tptr;
	    printf("%sIPv4 Prefix %s/%u-%u, origin-as %u, flags 0x%02x",
		   indent_string(indent+2),
		   ipaddr_string(pdu->prefix),
		   pdu->prefix_length, pdu->max_length,
		   EXTRACT_32BITS(pdu->as), pdu->flags);
	}
	break;

#ifdef INET6
    case RPKI_RTR_IPV6_PREFIX_PDU:
	{
	    rpki_rtr_pdu_ipv6_prefix *pdu;

	    pdu = (rpki_rtr_pdu_ipv6_prefix *)tptr;
	    printf("%sIPv6 Prefix %s/%u-%u, origin-as %u, flags 0x%02x",
		   indent_string(indent+2),
		   ip6addr_string(pdu->prefix),
		   pdu->prefix_length, pdu->max_length,
		   EXTRACT_32BITS(pdu->as), pdu->flags);
	}
	break;
#endif

    case RPKI_RTR_ERROR_REPORT_PDU:
	{
	    rpki_rtr_pdu_error_report *pdu;
	    u_int encapsulated_pdu_length, text_length, tlen, error_code;
	    u_char buf[80];

	    pdu = (rpki_rtr_pdu_error_report *)tptr;
	    encapsulated_pdu_length = EXTRACT_32BITS(pdu->encapsulated_pdu_length);
	    tlen = pdu_len;

	    error_code = EXTRACT_16BITS(pdu->pdu_header.u.error_code);
	    printf("%sError code: %s (%u), Encapsulated PDU length: %u",
		   indent_string(indent+2),
		   tok2str(rpki_rtr_error_codes, "Unknown", error_code),
		   error_code, encapsulated_pdu_length);

	    tptr += sizeof(*pdu);
	    tlen -= sizeof(*pdu);

	    /*
	     * Recurse if there is an encapsulated PDU.
	     */
	    if (encapsulated_pdu_length &&
		(encapsulated_pdu_length <= tlen)) {
		printf("%s-----encapsulated PDU-----", indent_string(indent+4));
		rpki_rtr_pdu_print(tptr, indent+2);
	    }

	    tptr += encapsulated_pdu_length;
	    tlen -= encapsulated_pdu_length;

	    /*
	     * Extract, trail-zero and print the Error message.
	     */ 
	    text_length = 0;
	    if (tlen > 4) {
		text_length = EXTRACT_32BITS(tptr);
		tptr += 4;
		tlen -= 4;
	    }
	    if (text_length && (text_length <= tlen )) {
		memcpy(buf, tptr, MIN(sizeof(buf)-1, text_length));
		buf[text_length] = '\0';
		printf("%sError text: %s", indent_string(indent+2), buf);
	    }
	}
	break;

    default:

	/*
	 * Unknown data, please hexdump.
	 */ 
	hexdump = TRUE;
    }

    /* do we also want to see a hex dump ? */
    if (vflag > 1 || (vflag && hexdump)) {
	print_unknown_data(tptr,"\n\t  ", pdu_len);
    }
}

void
rpki_rtr_print(register const u_char *pptr, register u_int len) {

    u_int tlen, pdu_type, pdu_len;
    const u_char *tptr;
    const rpki_rtr_pdu *pdu_header;
    
    tptr = pptr;
    tlen = len;

    if (!vflag) {
	printf(", RPKI-RTR");
	return;
    }

    while (tlen >= sizeof(rpki_rtr_pdu)) {

        TCHECK2(*tptr, sizeof(rpki_rtr_pdu));

	pdu_header = (rpki_rtr_pdu *)tptr;
        pdu_type = pdu_header->pdu_type;
        pdu_len = EXTRACT_32BITS(pdu_header->length);

        /* infinite loop check */
        if (!pdu_type || !pdu_len) {
            break;
        }

        TCHECK2(*tptr, pdu_len);
        if (tlen < pdu_len) {
            goto trunc;
        }

	/*
	 * Print the PDU.
	 */
	rpki_rtr_pdu_print(tptr, 8);

        tlen -= pdu_len;
        tptr += pdu_len;
    }
    return;
 trunc:
    printf("\n\t[|RPKI-RTR]");
}

/*
 * Local Variables:
 * c-style: whitesmith
 * c-basic-offset: 4
 * End:
 */