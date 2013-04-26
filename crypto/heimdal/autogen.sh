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
# to really generate all files you need to run "make distcheck" in a
# object tree, but this will do if you have all parts of the required
# tool-chain installedautoreconf -f -i || { echo "autoreconf failed: $?"; exit 1; }
find . \( -name '*-private.h' -o -name '*-protos.h' \) -delete