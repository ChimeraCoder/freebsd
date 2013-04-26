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

# Delete a specified partition, takes effect immediately
########################################################

. ${PROGDIR}/backend/functions.sh
. ${PROGDIR}/backend/functions-disk.sh

if [ -z "${1}" ]
then
  echo "Error: No partition specified!"
  exit 1
fi

if [ ! -e "/dev/${1}" ]
then
  echo "Error: Partition /dev/${1} does not exist!"
  exit 1
fi

PARTITION="${1}"

# First lets figure out the partition number for the given device
##################################################################

# Get the number of characters in this dev
CHARS="`echo $PARTITION | wc -c`"

PARTINDEX=""

# Lets read through backwards until we get the part number
while 
z=1
do
  CHARS=$((CHARS-1))
  LAST_CHAR=`echo "${PARTITION}" | cut -c $CHARS`
  echo "${LAST_CHAR}" | grep -q "^[0-9]$" 2>/dev/null
  if [ $? -eq 0 ] ; then
    PARTINDEX="${LAST_CHAR}${PARTINDEX}"
  else
    break
  fi
done

# Now get current disk we are working on
CHARS=`expr $CHARS - 1`
DISK="`echo $PARTITION | cut -c 1-${CHARS}`"

# Make sure we have a valid disk name still
if [ ! -e "/dev/${DISK}" ] ; then
  echo "Error: Disk: ${DISK} doesn't exist!"
  exit 1
fi

echo "Running: gpart delete -i ${PARTINDEX} ${DISK}"
gpart delete -i ${PARTINDEX} ${DISK} >/dev/null 2>/dev/null

# Check if this was the last partition and destroy the disk geom if so
get_disk_partitions "${DISK}"
if [ -z "${VAL}" ] ; then
  gpart destroy ${DISK}  
fi

exit "$?"