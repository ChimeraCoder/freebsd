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

FOUND="0"

# Lets parse the xorg.list file, and see what varients are supported
while read line
do

  if [ "$FOUND" = "1" -a ! -z "$line" ]
  then
    echo $line | grep '! ' >/dev/null 2>/dev/null
    if [ "$?" = "0" ]
    then
      exit 0
    else 
      echo "$line"
    fi 
  fi 

  if [ "${FOUND}" = "0" ]
  then
    echo $line | grep '! variant' >/dev/null 2>/dev/null
    if [ "$?" = "0" ]
    then
      FOUND="1"
    fi 
  fi

done < /usr/local/share/X11/xkb/rules/xorg.lst

exit 0