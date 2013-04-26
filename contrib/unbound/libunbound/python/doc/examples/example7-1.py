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
# vim:fileencoding=utf-8
#
# IDN (Internationalized Domain Name) lookup support
#import unbound

ctx = unbound.ub_ctx()
ctx.resolvconf("/etc/resolv.conf")

status, result = ctx.resolve(u"www.háčkyčárky.cz", unbound.RR_TYPE_A, unbound.RR_CLASS_IN)
if status == 0 and result.havedata:
    print "Result:"
    print "      raw data:", result.data
    for k in result.data.address_list:
        print "      address:%s" % k