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

# Script which lists the available packages for this release
###########################################################################

. ${PROGDIR}/backend/functions.sh
. ${PROGDIR}/backend/functions-packages.sh

ID=`id -u`
if [ "${ID}" -ne "0" ]
then
  echo "Error: must be root!" 
  exit 1
fi

if [ ! -f "${PKGDIR}/INDEX" ]
then
  get_package_index
fi

if [ -f "${PKGDIR}/INDEX" ]
then
  echo "${PKGDIR}/INDEX"
  exit 0
fi

exit 1