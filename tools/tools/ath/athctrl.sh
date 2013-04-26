#! /bin/sh
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
#
# Set the IFS parameters for an interface configured for
# point-to-point use at a specific distance.  Based on a
# program by Gunter Burchardt.
#
# $FreeBSD$
#DEV=ath0
d=0

usage()
{
	echo "Usage: $0 [-i athX] [-d meters]"
	exit 2
}

args=`getopt d:i: $*`
test $? -ne 0 && usage

set -- $args
for i; do
	case "$i" in
	-i)	DEV="$2"; shift; shift;;
	-d)	d="$2"; shift; shift;;
	--)	shift; break;
	esac
done

test $d -eq 0 && usage

slottime=`expr 9 + \( $d / 300 \)`
if expr \( $d % 300 \) != 0 >/dev/null 2>&1; then
	slottime=`expr $slottime + 1`
fi
timeout=`expr $slottime \* 2 + 3`

printf "Setup IFS parameters on interface ${DEV} for %i meter p-2-p link\n" $d
ATHN=`echo $DEV | sed 's/ath//'`
sysctl dev.ath.$ATHN.slottime=$slottime
sysctl dev.ath.$ATHN.acktimeout=$timeout
sysctl dev.ath.$ATHN.ctstimeout=$timeout