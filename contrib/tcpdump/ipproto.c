
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
    "@(#) $Header: /tcpdump/master/tcpdump/ipproto.c,v 1.6 2005-09-20 06:01:22 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include "interface.h"
#include "ipproto.h"

const struct tok ipproto_values[] = {
    { IPPROTO_HOPOPTS, "Options" },
    { IPPROTO_ICMP, "ICMP" },
    { IPPROTO_IGMP, "IGMP" },
    { IPPROTO_IPV4, "IPIP" },
    { IPPROTO_TCP, "TCP" },
    { IPPROTO_EGP, "EGP" },
    { IPPROTO_PIGP, "IGRP" },
    { IPPROTO_UDP, "UDP" },
    { IPPROTO_DCCP, "DCCP" },
    { IPPROTO_IPV6, "IPv6" },
    { IPPROTO_ROUTING, "Routing" },
    { IPPROTO_FRAGMENT, "Fragment" },
    { IPPROTO_RSVP, "RSVP" },
    { IPPROTO_GRE, "GRE" },
    { IPPROTO_ESP, "ESP" },
    { IPPROTO_AH, "AH" },
    { IPPROTO_MOBILE, "Mobile IP" },
    { IPPROTO_ICMPV6, "ICMPv6" },
    { IPPROTO_MOBILITY_OLD, "Mobile IP (old)" },
    { IPPROTO_EIGRP, "EIGRP" },
    { IPPROTO_OSPF, "OSPF" },
    { IPPROTO_PIM, "PIM" },
    { IPPROTO_IPCOMP, "Compressed IP" },
    { IPPROTO_VRRP, "VRRP" },
    { IPPROTO_PGM, "PGM" },
    { IPPROTO_SCTP, "SCTP" },
    { IPPROTO_MOBILITY, "Mobility" },
    { IPPROTO_CARP, "CARP" },
    { IPPROTO_PFSYNC, "pfsync" },
    { 0, NULL }
};