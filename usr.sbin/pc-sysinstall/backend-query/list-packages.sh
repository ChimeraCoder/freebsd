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

PACKAGE_CATEGORY="${1}"
PACKAGE_NAME="${2}"
NARGS=0

if [ ! -f "${PKGDIR}/INDEX" ]
then
  echo "Error: please fetch package index with get-packages!"
  exit 1
fi

if [ ! -f "${PKGDIR}/INDEX.parsed" ]
then
  parse_package_index
fi

if [ -n "${PACKAGE_CATEGORY}" ]
then
  NARGS=$((NARGS+1))
fi

if [ -n "${PACKAGE_NAME}" ]
then
  NARGS=$((NARGS+1))
fi

if [ "${NARGS}" -eq "0" ]
then
  show_packages

elif [ "${NARGS}" -eq "1" ]
then
	
  if [ "${PACKAGE_CATEGORY}" = "@INDEX@" ]
  then
    if [ -f "${PKGDIR}/INDEX" ]
    then
      echo "${PKGDIR}/INDEX"
      exit 0
    else
      exit 1
    fi
		
  else
    show_packages_by_category "${PACKAGE_CATEGORY}"
  fi

elif [ "${NARGS}" -eq "2" ]
then
  show_package_by_name "${PACKAGE_CATEGORY}" "${PACKAGE_NAME}"

else
  show_packages
fi