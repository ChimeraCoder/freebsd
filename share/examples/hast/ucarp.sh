#!/bin/sh
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

# Shared IP address, unused for now.
addr="10.99.0.3"
# Password for UCARP communication.
pass="password"
# First node IP and interface for UCARP communication.
nodea_srcip="10.99.0.1"
nodea_ifnet="bge0"
# Second node IP and interface for UCARP communication.
nodeb_srcip="10.99.0.2"
nodeb_ifnet="em3"

export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin

vhid="1"
upscript="/root/hast/sbin/hastd/vip-up.sh"
downscript="/root/hast/sbin/hastd/vip-down.sh"

ifconfig "${nodea_ifnet}" 2>/dev/null | grep -q "inet ${nodea_srcip} "
if [ $? -eq 0 ]; then
	srcip="${nodea_srcip}"
	ifnet="${nodea_ifnet}"
	node="node A"
fi
ifconfig "${nodeb_ifnet}" 2>/dev/null | grep -q "inet ${nodeb_srcip} "
if [ $? -eq 0 ]; then
	if [ -n "${srcip}" -o -n "${ifnet}" ]; then
		echo "Unable to determine which node is this (both match)." >/dev/stderr
		exit 1
	fi
	srcip="${nodeb_srcip}"
	ifnet="${nodeb_ifnet}"
	node="node B"
fi
if [ -z "${srcip}" -o -z "${ifnet}" ]; then
	echo "Unable to determine which node is this (none match)." >/dev/stderr
	exit 1
fi
ucarp -i ${ifnet} -s ${srcip} -v ${vhid} -a ${addr} -p ${pass} -u "${upscript}" -d "${downscript}"