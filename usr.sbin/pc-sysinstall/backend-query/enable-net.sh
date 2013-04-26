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

# Script which enables networking with specified options
###########################################################################

. ${PROGDIR}/backend/functions.sh
. ${PROGDIR}/conf/pc-sysinstall.conf
. ${BACKEND}/functions-networking.sh
. ${BACKEND}/functions-parse.sh


NIC="$1"
IP="$2"
NETMASK="$3"
DNS="$4"
GATEWAY="$5"
MIRRORFETCH="$6"
IPV6="$7"
IPV6GATE="$8"
IPV6DNS="$9"

if [ -z "${NIC}" ]
then
  echo "ERROR: Usage enable-net <nic> <ip> <netmask> <dns> <gateway> <ipv6> " \
	"<ipv6gateway> <ipv6dns>"
  exit 150
fi

if [ "$NIC" = "AUTO-DHCP" ]
then
  enable_auto_dhcp
elif [ "$NIC" = "IPv6-SLAAC" ]
then
  enable_auto_slaac
  # In addition, if static values were defined, add them as well.
  # We might not get DNS information from RAs, for example.
  if [ -n "${IPV6}" ]; then
    VAL=""
    get_first_wired_nic
    if [ -n "${VAL}" ]; then
      ifconfig ${VAL} inet6 ${IPV6} alias
    fi
  fi
  # Append only here.
  if [ -n "${IPV6DNS}" ]; then
    echo "nameserver ${IPV6DNS}" >>/etc/resolv.conf
  fi
  # Do not 
  if [ -n "${IPV6GATE}" ]; then
    # Check if we have a default route already to not overwrite.
    if ! route -n get -inet6 default > /dev/null 2>&1 ; then
      route add -inet6 default ${IPV6GATE}
    fi
  fi
else
  echo "Enabling NIC: $NIC"
  if [ -n "${IP}" ]; then
    ifconfig ${NIC} inet ${IP} ${NETMASK}
  fi
  if [ -n "${IPV6}" ]; then
    ifconfig ${NIC} inet6 ${IPV6} alias
  fi

  # Keep default from IPv4-only support times and clear the resolv.conf file.
  : > /etc/resolv.conf
  if [ -n "${DNS}" ]; then
    echo "nameserver ${DNS}" >>/etc/resolv.conf
  fi
  if [ -n "${IPV6DNS}" ]; then
    echo "nameserver ${IPV6DNS}" >>/etc/resolv.conf
  fi

  if [ -n "${GATE}" ]; then
    route add -inet default ${GATE}
  fi
  if [ -n "${IPV6GATE}" ]; then
    route add -inet6 default ${IPV6GATE}
  fi
fi

case ${MIRRORFETCH} in
  ON|on|yes|YES) fetch -o /tmp/mirrors-list.txt ${MIRRORLIST} >/dev/null 2>/dev/null;;
  *) ;;
esac