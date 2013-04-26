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
echo "/*"
echo " * hashsize.h -- hash and token table constants"
echo " */"

CAPS="${1-Caps}"
TABSIZE=`grep -v '^[ #]' $CAPS | grep -v "^$" | grep -v "^capalias"| grep -v "^infoalias" | wc -l`

echo ""
echo "#define CAPTABSIZE	${TABSIZE}"
echo "#define HASHTABSIZE	(${TABSIZE} * 2)"