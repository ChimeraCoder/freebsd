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
# $FreeBSD$
#
mntpoint="/mnt/test-1"

#
# prepare
kldload geom_uzip
uudecode test-1.img.uzip.uue
num=`mdconfig -an -f test-1.img.uzip` || exit 1
sleep 1

#
# mount
mkdir -p "${mntpoint}"
mount -o ro /dev/md${num}.uzip "${mntpoint}" || exit 1

#
# compare
#cat "${mntpoint}/etalon.txt"
diff -u etalon/etalon.txt "${mntpoint}/etalon.txt"
if [ $? -eq 0 ]; then
	echo "PASS"
else
	echo "FAIL"
fi

#
# cleanup
umount "${mntpoint}"
rmdir "${mntpoint}"
mdconfig -d -u ${num}
sleep 1
kldunload geom_uzip