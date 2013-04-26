
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
    "@(#) $Header: /tcpdump/master/tcpdump/missing/dlnames.c,v 1.5 2003-11-18 23:09:43 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <pcap.h>
#include <string.h>

#include "pcap-missing.h"

struct dlt_choice {
	const char *name;
	const char *description;
	int	dlt;
};

#define DLT_CHOICE(code, description) { #code, description, code }
#define DLT_CHOICE_SENTINEL { NULL, NULL, 0 }

static struct dlt_choice dlt_choices[] = {
	DLT_CHOICE(DLT_NULL, "BSD loopback"),
	DLT_CHOICE(DLT_EN10MB, "Ethernet"),
	DLT_CHOICE(DLT_IEEE802, "Token ring"),
	DLT_CHOICE(DLT_ARCNET, "ARCNET"),
	DLT_CHOICE(DLT_SLIP, "SLIP"),
	DLT_CHOICE(DLT_PPP, "PPP"),
	DLT_CHOICE(DLT_FDDI, "FDDI"),
	DLT_CHOICE(DLT_ATM_RFC1483, "RFC 1483 IP-over-ATM"),
	DLT_CHOICE(DLT_RAW, "Raw IP"),
#ifdef DLT_SLIP_BSDOS
	DLT_CHOICE(DLT_SLIP_BSDOS, "BSD/OS SLIP"),
#endif
#ifdef DLT_PPP_BSDOS
	DLT_CHOICE(DLT_PPP_BSDOS, "BSD/OS PPP"),
#endif
#ifdef DLT_ATM_CLIP
	DLT_CHOICE(DLT_ATM_CLIP, "Linux Classical IP-over-ATM"),
#endif
#ifdef DLT_PPP_SERIAL
	DLT_CHOICE(DLT_PPP_SERIAL, "PPP over serial"),
#endif
#ifdef DLT_PPP_ETHER
	DLT_CHOICE(DLT_PPP_ETHER, "PPPoE"),
#endif
#ifdef DLT_C_HDLC
	DLT_CHOICE(DLT_C_HDLC, "Cisco HDLC"),
#endif
#ifdef DLT_IEEE802_11
	DLT_CHOICE(DLT_IEEE802_11, "802.11"),
#endif
#ifdef DLT_FRELAY
	DLT_CHOICE(DLT_FRELAY, "Frame Relay"),
#endif
#ifdef DLT_LOOP
	DLT_CHOICE(DLT_LOOP, "OpenBSD loopback"),
#endif
#ifdef DLT_ENC
	DLT_CHOICE(DLT_ENC, "OpenBSD encapsulated IP"),
#endif
#ifdef DLT_LINUX_SLL
	DLT_CHOICE(DLT_LINUX_SLL, "Linux cooked"),
#endif
#ifdef DLT_LTALK
	DLT_CHOICE(DLT_LTALK, "Localtalk"),
#endif
#ifdef DLT_PFLOG
	DLT_CHOICE(DLT_PFLOG, "OpenBSD pflog file"),
#endif
#ifdef DLT_PRISM_HEADER
	DLT_CHOICE(DLT_PRISM_HEADER, "802.11 plus Prism header"),
#endif
#ifdef DLT_IP_OVER_FC
	DLT_CHOICE(DLT_IP_OVER_FC, "RFC 2625 IP-over-Fibre Channel"),
#endif
#ifdef DLT_SUNATM
	DLT_CHOICE(DLT_SUNATM, "Sun raw ATM"),
#endif
#ifdef DLT_IEEE802_11_RADIO
	DLT_CHOICE(DLT_IEEE802_11_RADIO, "802.11 plus radio information header"),
#endif
#ifdef DLT_ARCNET_LINUX
	DLT_CHOICE(DLT_ARCNET_LINUX, "Linux ARCNET"),
#endif
#ifdef DLT_LINUX_IRDA
	DLT_CHOICE(DLT_LINUX_IRDA, "Linux IrDA"),
#endif
#ifdef DLT_LANE8023
	DLT_CHOICE(DLT_LANE8023, "Linux 802.3 LANE"),
#endif
#ifdef DLT_CIP
	DLT_CHOICE(DLT_CIP, "Linux Classical IP-over-ATM"),
#endif
#ifdef DLT_HDLC
	DLT_CHOICE(DLT_HDLC, "Cisco HDLC"),
#endif
	DLT_CHOICE_SENTINEL
};

#ifndef HAVE_PCAP_DATALINK_NAME_TO_VAL
int
pcap_datalink_name_to_val(const char *name)
{
	int i;

	for (i = 0; dlt_choices[i].name != NULL; i++) {
		if (strcasecmp(dlt_choices[i].name + sizeof("DLT_") - 1,
		    name) == 0)
			return (dlt_choices[i].dlt);
	}
	return (-1);
}

const char *
pcap_datalink_val_to_name(int dlt)
{
	int i;

	for (i = 0; dlt_choices[i].name != NULL; i++) {
		if (dlt_choices[i].dlt == dlt)
			return (dlt_choices[i].name + sizeof("DLT_") - 1);
	}
	return (NULL);
}
#endif

const char *
pcap_datalink_val_to_description(int dlt)
{
	int i;

	for (i = 0; dlt_choices[i].name != NULL; i++) {
		if (dlt_choices[i].dlt == dlt)
			return (dlt_choices[i].description);
	}
	return (NULL);
}