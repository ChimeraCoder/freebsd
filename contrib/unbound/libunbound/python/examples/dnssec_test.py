#!/usr/bin/env python
# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
from unbound import ub_ctx, RR_TYPE_A, RR_TYPE_RRSIG, RR_TYPE_NSEC, RR_TYPE_NSEC3
import ldns

def dnssecParse(domain, rrType=RR_TYPE_A):
    print "Resolving domain", domain
    s, r = resolver.resolve(domain)
    print "status: %s, secure: %s, rcode: %s, havedata: %s, answer_len; %s" % (s, r.secure, r.rcode_str, r.havedata, r.answer_len)
    
    s, pkt = ldns.ldns_wire2pkt(r.packet)
    if s != 0:
        raise RuntimeError("Error parsing DNS packet")

    rrsigs = pkt.rr_list_by_type(RR_TYPE_RRSIG, ldns.LDNS_SECTION_ANSWER)
    print "RRSIGs from answer:", rrsigs
    
    rrsigs = pkt.rr_list_by_type(RR_TYPE_RRSIG, ldns.LDNS_SECTION_AUTHORITY)
    print "RRSIGs from authority:", rrsigs
    
    nsecs = pkt.rr_list_by_type(RR_TYPE_NSEC, ldns.LDNS_SECTION_AUTHORITY)
    print "NSECs:", nsecs
    
    nsec3s = pkt.rr_list_by_type(RR_TYPE_NSEC3, ldns.LDNS_SECTION_AUTHORITY)
    print "NSEC3s:", nsec3s
    
    print "---"


resolver = ub_ctx()
resolver.add_ta(".   IN DS   19036 8 2 49AAC11D7B6F6446702E54A1607371607A1A41855200FD2CE1CDDE32F24E8FB5")

dnssecParse("nic.cz")
dnssecParse("nonexistent-domain-blablabla.cz")
dnssecParse("nonexistent-domain-blablabla.root.cz")