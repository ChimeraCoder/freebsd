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
#
# This script generates a "memstick image" (image that can be copied to a
# USB memory stick) from a directory tree.  Note that the script does not
# clean up after itself very well for error conditions on purpose so the
# problem can be diagnosed (full filesystem most likely but ...).
#
# Usage: make-memstick.sh <directory tree> <image filename>
#
# $FreeBSD$
#
PATH=/bin:/usr/bin:/sbin:/usr/sbin
export PATH

if [ $# -ne 2 ]; then
  echo "make-memstick.sh /path/to/directory /path/to/image/file"
  exit 1
fi

if [ ! -d ${1} ]; then
  echo "${1} must be a directory"
  exit 1
fi

if [ -e ${2} ]; then
  echo "won't overwrite ${2}"
  exit 1
fi

echo '/dev/ufs/FreeBSD_Install / ufs ro,noatime 1 1' > ${1}/etc/fstab
makefs -B little -o label=FreeBSD_Install ${2} ${1}
if [ $? -ne 0 ]; then
  echo "makefs failed"
  exit 1
fi
rm ${1}/etc/fstab

unit=`mdconfig -a -t vnode -f ${2}`
if [ $? -ne 0 ]; then
  echo "mdconfig failed"
  exit 1
fi
gpart create -s BSD ${unit}
gpart bootcode -b ${1}/boot/boot ${unit}
gpart add -t freebsd-ufs ${unit}
mdconfig -d -u ${unit}