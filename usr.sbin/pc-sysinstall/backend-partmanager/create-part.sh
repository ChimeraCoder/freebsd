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

# Create partitions on a target disk
#############################

. ${PROGDIR}/backend/functions.sh

if [ -z "${1}" ] ; then
  echo "Error: No disk specified!"
  exit 1
fi

if [ -z "${2}" ] ; then
  echo "Error: No size specified!"
  exit 1
fi

if [ ! -e "/dev/${1}" ] ; then
  echo "Error: Disk /dev/${1} does not exist!"
  exit 1
fi

DISK="${1}"
MB="${2}"
TYPE="${3}"
STARTBLOCK="${4}"

TOTALBLOCKS="`expr $MB \* 2048`"

# If no TYPE specified, default to MBR
if [ -z "$TYPE" ] ; then TYPE="mbr" ; fi

# Sanity check the gpart type
case $TYPE in
	apm|APM) ;;
	bsd|BSD) ;;
	ebr|EBR) ;;
      pc98|pc98) ;;
	gpt|GPT) ;;
	mbr|MBR) ;;
    vtoc8|VTOC8) ;;
	*) echo "Error: Unknown gpart type: $TYPE" ; exit 1 ;;
esac

# Lets figure out what number this partition will be
LASTSLICE="`gpart show $DISK | grep -v -e $DISK -e '\- free \-' -e '^$' | awk 'END {print $3}'`"
if [ -z "${LASTSLICE}" ] ; then
  LASTSLICE="1"
else
  LASTSLICE="`expr $LASTSLICE + 1`"
fi

SLICENUM="${LASTSLICE}"

# Set a 4k Aligned start block if none specified
if [ "${SLICENUM}" = "1" -a -z "$STARTBLOCK" ] ; then
  STARTBLOCK="2016"
fi


# If this is an empty disk, see if we need to create a new scheme for it
gpart show ${DISK} >/dev/null 2>/dev/null
if [ $? -eq 0 -a "${SLICENUM}" = "1" ] ; then
  if [ "${TYPE}" = "mbr" -o "${TYPE}" = "MBR" ] ; then 
    flags="-s ${TYPE} -f active"
  else
    flags="-s ${TYPE}"
  fi
  gpart create ${flags} ${DISK}
fi

# If we have a starting block, use it
if [ -n "$STARTBLOCK" ] ; then
  sBLOCK="-b $STARTBLOCK"
fi

gpart add ${sBLOCK} -s ${TOTALBLOCKS} -t freebsd -i ${SLICENUM} ${DISK}
exit "$?"