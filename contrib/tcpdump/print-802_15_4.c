
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <pcap.h>
#include <string.h>

#include "interface.h"
#include "addrtoname.h"

#include "extract.h"

static const char *ftypes[] = {
	"Beacon",			/* 0 */
	"Data",				/* 1 */
	"ACK",				/* 2 */
	"Command",			/* 3 */
	"Reserved",			/* 4 */
	"Reserved",			/* 5 */
	"Reserved",			/* 6 */
	"Reserved",			/* 7 */
};

static int
extract_header_length(u_int16_t fc)
{
	int len = 0;

	switch ((fc >> 10) & 0x3) {
	case 0x00:
		if (fc & (1 << 6)) /* intra-PAN with none dest addr */
			return -1;
		break;
	case 0x01:
		return -1;
	case 0x02:
		len += 4;
		break;
	case 0x03:
		len += 10;
		break;
	}

	switch ((fc >> 14) & 0x3) {
	case 0x00:
		break;
	case 0x01:
		return -1;
	case 0x02:
		len += 4;
		break;
	case 0x03:
		len += 10;
		break;
	}

	if (fc & (1 << 6)) {
		if (len < 2)
			return -1;
		len -= 2;
	}

	return len;
}


u_int
ieee802_15_4_if_print(struct netdissect_options *ndo,
                      const struct pcap_pkthdr *h, const u_char *p)
{
	u_int caplen = h->caplen;
	int hdrlen;
	u_int16_t fc;
	u_int8_t seq;

	if (caplen < 3) {
		ND_PRINT((ndo, "[|802.15.4] %x", caplen));
		return caplen;
	}

	fc = EXTRACT_LE_16BITS(p);
	hdrlen = extract_header_length(fc);

	seq = EXTRACT_LE_8BITS(p + 2);

	p += 3;
	caplen -= 3;

	ND_PRINT((ndo,"IEEE 802.15.4 %s packet ", ftypes[fc & 0x7]));
	if (vflag)
		ND_PRINT((ndo,"seq %02x ", seq));
	if (hdrlen == -1) {
		ND_PRINT((ndo,"malformed! "));
		return caplen;
	}


	if (!vflag) {
		p+= hdrlen;
		caplen -= hdrlen;
	} else {
		u_int16_t panid = 0;

		switch ((fc >> 10) & 0x3) {
		case 0x00:
			ND_PRINT((ndo,"none "));
			break;
		case 0x01:
			ND_PRINT((ndo,"reserved destination addressing mode"));
			return 0;
		case 0x02:
			panid = EXTRACT_LE_16BITS(p);
			p += 2;
			ND_PRINT((ndo,"%04x:%04x ", panid, EXTRACT_LE_16BITS(p)));
			p += 2;
			break;
		case 0x03:
			panid = EXTRACT_LE_16BITS(p);
			p += 2;
			ND_PRINT((ndo,"%04x:%s ", panid, le64addr_string(p)));
			p += 8;
			break;
		}
		ND_PRINT((ndo,"< ");

		switch ((fc >> 14) & 0x3) {
		case 0x00:
			ND_PRINT((ndo,"none "));
			break;
		case 0x01:
			ND_PRINT((ndo,"reserved source addressing mode"));
			return 0;
		case 0x02:
			if (!(fc & (1 << 6))) {
				panid = EXTRACT_LE_16BITS(p);
				p += 2;
			}
			ND_PRINT((ndo,"%04x:%04x ", panid, EXTRACT_LE_16BITS(p)));
			p += 2;
			break;
		case 0x03:
			if (!(fc & (1 << 6))) {
				panid = EXTRACT_LE_16BITS(p);
				p += 2;
			}
                        ND_PRINT((ndo,"%04x:%s ", panid, le64addr_string(p))));
			p += 8;
			break;
		}

		caplen -= hdrlen;
	}

	if (!suppress_default_print)
		(ndo->ndo_default_print)(ndo, p, caplen);

	return 0;
}