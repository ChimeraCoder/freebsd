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


# Script which tries to ping "home" to see if Internet connectivity is
# available.
#############################################################################

rm ${TMPDIR}/.testftp >/dev/null 2>/dev/null

ping -c 2 www.pcbsd.org >/dev/null 2>/dev/null
if [ "$?" = "0" ]
then
  echo "ftp: Up"
  exit 0
fi

ping6 -c 2 www.pcbsd.org >/dev/null 2>/dev/null
if [ "$?" = "0" ]
then
  echo "ftp: Up"
  exit 0
fi

ping -c 2 www.freebsd.org >/dev/null 2>/dev/null
if [ "$?" = "0" ]
then
  echo "ftp: Up"
  exit 0
fi

ping6 -c 2 www.freebsd.org >/dev/null 2>/dev/null
if [ "$?" = "0" ]
then
  echo "ftp: Up"
  exit 0
fi

echo "ftp: Down"
exit 1