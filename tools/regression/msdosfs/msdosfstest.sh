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
# $FreeBSD$
# A really simple script to create a swap-backed msdosfs filesystem, copy a few
# files to it, unmount/remount the filesystem, and make sure all is well.
# 
# Not very advanced, but better than nothing. mkdir /tmp/msdosfstest/
mdconfig -a -t swap -s 128m -u 10
bsdlabel -w md10 auto
newfs_msdos -F 16 -b 8192 /dev/md10a
mount_msdosfs /dev/md10a /tmp/msdosfstest/
cp -R /usr/src/bin/ /tmp/msdosfstest/
umount /tmp/msdosfstest/
mount_msdosfs /dev/md10a /tmp/msdosfstest/
diff -u -r /usr/src/bin /tmp/msdosfstest/
if [ $? -eq 0 ]; then
	echo "ok 1";
else
	echo "not ok 1";
fi
umount /tmp/msdosfstest/
mdconfig -d -u 10
rmdir /tmp/msdosfstest/