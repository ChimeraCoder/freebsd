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

# Script which lists the available components for this release
###########################################################################

. ${PROGDIR}/backend/functions.sh

echo "Available Components:"

if [ -d "${COMPDIR}" ]
then
  cd ${COMPDIR}
  for i in `ls -d *`
  do
    if [ -e "${i}/component.cfg" -a -e "${i}/install.sh" -a -e "${i}/distfiles" ]
    then
      NAME="`grep 'name:' ${i}/component.cfg | cut -d ':' -f 2`"
      DESC="`grep 'description:' ${i}/component.cfg | cut -d ':' -f 2`"
      TYPE="`grep 'type:' ${i}/component.cfg | cut -d ':' -f 2`"
      echo " "
      echo "name: ${i}"
      echo "desc:${DESC}"
      echo "type:${TYPE}"
      if [ -e "${i}/component.png" ]
      then
        echo "icon: ${COMPDIR}/${i}/component.png"
      fi
    fi
  done
fi