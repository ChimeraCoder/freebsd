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

. ${PROGDIR}/backend/functions.sh
. ${PROGDIR}/backend/functions-ftp.sh

# Backend script which lists all the available ftp mirrors for front-ends to display
COUNTRY="${1}"

get_ftp_mirrors "${COUNTRY}"
show_mirrors "${VAL}"

exit 0