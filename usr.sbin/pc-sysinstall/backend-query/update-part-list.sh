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

# Need access to a some unmount functions
. ${PROGDIR}/backend/functions-unmount.sh

echo "Running: find-update-parts" >> ${LOGOUT}

rm ${TMPDIR}/AvailUpgrades >/dev/null 2>/dev/null

FSMNT="/mnt"

# Get the freebsd version on this partition
get_fbsd_ver()
{
  sFiles="/bin/sh /boot/kernel/kernel"
  for file in $sFiles
  do
     if [ ! -e "${FSMNT}/$file" ] ; then continue ; fi

     VER="`file ${FSMNT}/$file | grep 'for FreeBSD' | sed 's|for FreeBSD |;|g' | cut -d ';' -f 2 | cut -d ',' -f 1`"
    if [ "$?" = "0" ] ; then
      file ${FSMNT}/$file | grep '32-bit' >/dev/null 2>/dev/null
      if [ "${?}" = "0" ] ; then
        echo "${1}: FreeBSD ${VER} (32bit)"
      else
        echo "${1}: FreeBSD ${VER} (64bit)"
      fi
    fi
    break
  done

}

# Create our device listing
SYSDISK="`sysctl kern.disks | cut -d ':' -f 2 | sed 's/^[ \t]*//'`"
DEVS=""

# Now loop through these devices, and list the disk drives
for i in ${SYSDISK}
do

  # Get the current device
  DEV="${i}"
  # Make sure we don't find any cd devices
  echo "${DEV}" | grep -e "^acd[0-9]" -e "^cd[0-9]" -e "^scd[0-9]" >/dev/null 2>/dev/null
  if [ "$?" != "0" ] ; then
    DEVS="${DEVS} `ls /dev/${i}*`" 
  fi

done

# Search for regular UFS / Geom Partitions to upgrade
for i in $DEVS
do
  if [ ! -e "${i}a.journal" -a ! -e "${i}a" -a ! -e "${i}p2" -a ! -e "${i}p2.journal" ] ; then
    continue
  fi

  if [ -e "${i}a.journal" ] ; then
    _dsk="${i}a.journal" 
  elif [ -e "${i}a" ] ; then
    _dsk="${i}a" 
  elif [ -e "${i}p2" ] ; then
    _dsk="${i}p2" 
  elif [ -e "${i}p2.journal" ] ; then
    _dsk="${i}p2.journal" 
  fi

  mount -o ro ${_dsk} ${FSMNT} >>${LOGOUT} 2>>${LOGOUT}
  if [ $? -eq 0 ] ; then
    get_fbsd_ver "`echo ${_dsk} | sed 's|/dev/||g'`"
    umount -f ${FSMNT} >/dev/null 2>/dev/null
  fi
done