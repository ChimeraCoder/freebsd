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

# Script which creates a gzipped log and optionally mails it to the specified address
############################################################################

. ${PROGDIR}/backend/functions.sh
. ${PROGDIR}/conf/pc-sysinstall.conf
. ${BACKEND}/functions-networking.sh
. ${BACKEND}/functions-parse.sh

# Bring up all NICS under DHCP
enable_auto_dhcp

MAILTO="$1"
MAILRESULT="0"

# Set the location of our compressed log
TMPLOG="/tmp/pc-sysinstall.log"

echo "# PC-SYSINSTALL LOG" >${TMPLOG}
cat ${LOGOUT} >> ${TMPLOG}

# Check if we have a GUI generated install cfg
if [ -e "/tmp/sys-install.cfg" ]
then
  echo "" >>${TMPLOG}
  echo "# PC-SYSINSTALL CFG " >>${TMPLOG}
  cat /tmp/sys-install.cfg | grep -vE 'rootPass|userPass' >> ${TMPLOG}
fi

# Save dmesg output
echo "" >>${TMPLOG}
echo "# DMESG OUTPUT " >>${TMPLOG}
dmesg >> ${TMPLOG}

# Get gpart info on all disks
for i in `pc-sysinstall disk-list | cut -d ':' -f 1`
do
  echo "" >>${TMPLOG}
  echo "# DISK INFO $i " >>${TMPLOG}
  ls /dev/${i}* >>${TMPLOG}
  gpart show ${i} >> ${TMPLOG}
done

# Show Mounted volumes
echo "" >>${TMPLOG}
echo "# MOUNT OUTPUT " >>${TMPLOG}
mount >> ${TMPLOG}

echo "Log file saved to ${TMPLOG}"
echo "Warning: This file will be lost once the system is rebooted."

echo "Do you wish to view this logfile now? (Y/N)"
read tmp
if [ "$tmp" = "Y" -o "$tmp" = "y" ]
then
  more ${TMPLOG}
fi