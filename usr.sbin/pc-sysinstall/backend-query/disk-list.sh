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

ARGS=$1
FLAGS_MD=""
FLAGS_CD=""
FLAGS_VERBOSE=""

shift
while [ -n "$1" ]
do
  case "$1" in
    -m)
      FLAGS_MD=1
      ;;
    -v)
      FLAGS_VERBOSE=1
      ;;
    -c)
      FLAGS_CD=1
      ;;
  esac
  shift
done

# Create our device listing
SYSDISK=$(sysctl -n kern.disks)
if [ -n "${FLAGS_MD}" ]
then
  MDS=`mdconfig -l`
  if [ -n "${MDS}" ]
  then
    SYSDISK="${SYSDISK} ${MDS}"
  fi
fi

# Add any RAID devices
if [ -d "/dev/raid" ] ; then
  cd /dev/raid
  for i in `ls`
  do
      SYSDISK="${SYSDISK} ${i}"
  done
fi

# Now loop through these devices, and list the disk drives
for i in ${SYSDISK}
do

  # Get the current device
  DEV="${i}"

  # Make sure we don't find any cd devices
  if [ -z "${FLAGS_CD}" ]
  then
    case "${DEV}" in
      acd[0-9]*|cd[0-9]*|scd[0-9]*) continue ;;
    esac
  fi

  # Try and find some identification information with camcontrol
  NEWLINE=$(camcontrol identify $DEV 2>/dev/null | sed -ne 's/^device model *//p')
  if [ -z "$NEWLINE" ]; then
    	NEWLINE=" <Unknown Device>"
  fi

  if [ -n "${FLAGS_MD}" ] && echo "${DEV}" | grep -E '^md[0-9]+' >/dev/null 2>/dev/null
  then
	NEWLINE=" <Memory Disk>"
  fi

  if [ -n "${FLAGS_VERBOSE}" ]
  then
	:
  fi

  # Save the disk list
  if [ ! -z "$DLIST" ]
  then
    DLIST="\n${DLIST}"
  fi

  DLIST="${DEV}:${NEWLINE}${DLIST}"

done

# Echo out the found line
echo -e "$DLIST" | sort