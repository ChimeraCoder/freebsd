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

# Script which lists the backups present on a server
###########################################################################

. ${PROGDIR}/backend/functions.sh

SSHUSER=$1
SSHHOST=$2
SSHPORT=$3

if [ -z "${SSHHOST}" -o -z "${SSHPORT}" ]
then
  echo "ERROR: Usage list-rsync-backups.sh <user> <host> <port>"
  exit 150
fi

# Look for full-system backups, needs at minimum a kernel to be bootable
FINDCMD="find . -type d -maxdepth 6 -name 'kernel' | grep '/boot/kernel'"

# Get a listing of the number of full backups saved
OLDBACKUPS=`ssh -o 'BatchMode=yes' -p ${SSHPORT} ${SSHUSER}@${SSHHOST} "${FINDCMD}"`
if [ "$?" = "0" ]
then
  for i in ${OLDBACKUPS}
  do
    BACKPATH="`echo ${i} | sed 's|/boot/.*||g' | sed 's|^./||g'`"
    if [ -z "${BACKLIST}" ]
    then
      BACKLIST="${BACKPATH}"
    else
      BACKLIST="${BACKLIST}:${BACKPATH}"
    fi
  done

  if [ -z "${BACKLIST}" ]
  then
    echo "NONE"
  else
    echo "$BACKLIST"
  fi

else
  echo "FAILED"  
fi