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

if [ -f /etc/sysconfig/network ]; then
. /etc/sysconfig/network
fi

ib_sysfs="/sys/class/infiniband"
newname="$HOSTNAME"


function usage
{
	echo "Usage: `basename $0` [-hv] [<name>]"
	echo "   set the node_desc field of all hca's found in \"$ib_sysfs\""
	echo "   -h this help"
	echo "   -v view all node descriptors"
	echo "   [<name>] set name to name specified."
	echo "      Default is to use the hostname: \"$HOSTNAME\""
	exit 2
}

function viewall
{
   for hca in `ls $ib_sysfs`; do
      if [ -f $ib_sysfs/$hca/node_desc ]; then
         echo -n "$hca: "
         cat $ib_sysfs/$hca/node_desc
      else
         logger -s "Failed to set node_desc for : $hca"
      fi
   done
   exit 0
}

while getopts "hv" flag
do
   case $flag in
      "h") usage;;
      "v") viewall;;
   esac
done

shift $(($OPTIND - 1))

if [ "$1" != "" ]; then
	newname="$1"
fi

for hca in `ls $ib_sysfs`; do
   if [ -f $ib_sysfs/$hca/node_desc ]; then
      echo -n "$newname" >> $ib_sysfs/$hca/node_desc
   else
      logger -s "Failed to set node_desc for : $hca"
   fi
done

exit 0