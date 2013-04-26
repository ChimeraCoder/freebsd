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

# Query a disk for partitions and display them
#############################################################################

. ${PROGDIR}/backend/functions.sh
. ${PROGDIR}/backend/functions-disk.sh

DISK="${1}"

[ -z "${DISK}" ] && { echo 'Error: No disk specified!'; exit 1; }
[ ! -e "/dev/${DISK}" ] && \
	{ echo "Error: Disk /dev/${DISK} does not exist!"; exit 1; }

get_disk_cyl "${DISK}"
CYLS="${VAL}"

get_disk_heads "${DISK}"
HEADS="${VAL}"

get_disk_sectors "${DISK}"
SECS="${VAL}"

# Now get the disks size in MB
KB="`diskinfo -v ${1} | grep 'bytes' | cut -d '#' -f 1 | tr -s '\t' ' ' | tr -d ' '`"
MB=$(convert_byte_to_megabyte ${KB})

# Now get the Controller Type
CTYPE="`dmesg | grep "^${1}:" | grep "B <" | cut -d '>' -f 2 | cut -d ' ' -f 3-10`"

echo "cylinders=${CYLS}"
echo "heads=${HEADS}"
echo "sectors=${SECS}"
echo "size=${MB}"
echo "type=${CTYPE}"