#!/usr/bin/python
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
from unbound import ub_ctx,ub_strerror,RR_TYPE_A,RR_CLASS_IN

ctx = ub_ctx()
ctx.resolvconf("/etc/resolv.conf")
	
status, result = ctx.resolve("test.record.xxx", RR_TYPE_A, RR_CLASS_IN)
if status == 0 and result.havedata:
    print "Result:", result.data.address_list
else:
    print "No record found"

#define new local zone
status = ctx.zone_add("xxx.","static")
if (status != 0): print "Error zone_add:",status, ub_strerror(status)

#add RR to the zone
status = ctx.data_add("test.record.xxx. IN A 1.2.3.4")
if (status != 0): print "Error data_add:",status, ub_strerror(status)

#lookup for an A record
status, result = ctx.resolve("test.record.xxx", RR_TYPE_A, RR_CLASS_IN)
if status == 0 and result.havedata:
    print "Result:", result.data.as_address_list()
else:
    print "No record found"