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

obs=`strings -at d $1 | grep "MFS Filesystem goes here" | awk '{print $1}'`
dd if=$2 ibs=8192 of=$1 obs=${obs} oseek=1 conv=notrunc 2> /dev/null

strings $1 | grep 'MFS Filesystem had better STOP here' > /dev/null || \
	(rm $1 && echo "MFS image too large" && false)