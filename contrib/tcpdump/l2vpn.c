
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
    "@(#) $Header: /tcpdump/master/tcpdump/l2vpn.c,v 1.1 2004-06-15 09:42:40 hannes Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>
#include "interface.h"
#include "l2vpn.h"

/* draft-ietf-pwe3-iana-allocation-04 */
const struct tok l2vpn_encaps_values[] = {
    { 0x00, "Reserved"},
    { 0x01, "Frame Relay"},
    { 0x02, "ATM AAL5 VCC transport"},
    { 0x03, "ATM transparent cell transport"}, 
    { 0x04, "Ethernet VLAN"}, 
    { 0x05, "Ethernet"}, 
    { 0x06, "Cisco-HDLC"}, 
    { 0x07, "PPP"}, 
    { 0x08, "SONET/SDH Circuit Emulation Service over MPLS"}, 
    { 0x09, "ATM n-to-one VCC cell transport"}, 
    { 0x0a, "ATM n-to-one VPC cell transport"}, 
    { 0x0b, "IP Layer2 Transport"}, 
    { 0x0c, "ATM one-to-one VCC Cell Mode"}, 
    { 0x0d, "ATM one-to-one VPC Cell Mode"},
    { 0x0e, "ATM AAL5 PDU VCC transport"},
    { 0x0f, "Frame-Relay Port mode"},
    { 0x10, "SONET/SDH Circuit Emulation over Packet"},
    { 0x11, "Structure-agnostic E1 over Packet"},
    { 0x12, "Structure-agnostic T1 (DS1) over Packet"},
    { 0x13, "Structure-agnostic E3 over Packet"},
    { 0x14, "Structure-agnostic T3 (DS3) over Packet"},
    { 0x15, "CESoPSN basic mode"},
    { 0x16, "TDMoIP basic mode"},
    { 0x17, "CESoPSN TDM with CAS"},
    { 0x18, "TDMoIP TDM with CAS"},
    { 0x40, "IP-interworking"}, 
    { 0, NULL}
};