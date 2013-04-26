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
# Generate arm-tune.md, a file containing the tune attribute from the list of 
# CPUs in arm-cores.def
echo ";; -*- buffer-read-only: t -*-"
echo ";; Generated automatically by gentune.sh from arm-cores.def"

allcores=`awk -F'[(, 	]+' '/^ARM_CORE/ { cores = cores$3"," } END { print cores } ' $1`

echo "(define_attr \"tune\""
echo "	\"$allcores\"" | sed -e 's/,"$/"/'
echo "	(const (symbol_ref \"arm_tune\")))"