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

# $1: kernel filename
# $2: device tree blob filename

dtbsize=`ls -l $2 | awk '{print $5}'`
obs=`strings -at d $1 | grep "Device Tree Blob STARTS here" | awk '{print $1}'`
eol=`strings -at d $1 | grep "Device Tree Blob STOPS here" | awk '{print $1}'`
sz=$((eol - obs));
if [ $sz -lt $dtbsize ]
then
	echo "Selected blob is too big to reembed. Please rebuild the kernel."
	exit
fi
dd if=$2 ibs=${dtbsize} of=$1 obs=${obs} oseek=1 conv=notrunc 2> /dev/null