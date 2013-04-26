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
# A really simple script to create a swap-backed msdosfs filesystem, then
# test to make sure the mbnambuf optimisation(msdosfs_conv.c rev 1.40)
# doesn't break multi-byte characters.
mkdir /tmp/msdosfstest/
mdconfig -a -t swap -s 128m -u 10
bsdlabel -w md10 auto
newfs_msdos -F 16 -b 8192 /dev/md10a
mount_msdosfs -L zh_TW.Big5 -D CP950 /dev/md10a /tmp/msdosfstest/
# The comment is UTF-8, the actual command uses the Big5 representation.
# mkdir /tmp/msdosfstest/012345678_邪惡之美
mkdir /tmp/msdosfstest/012345678_$'\250\270\264\143\244\247\254\374'
cd /tmp/msdosfstest/012345678_$'\250\270\264\143\244\247\254\374'
if [ $? -eq 0 ]; then
	echo "ok 3";
else
	echo "not ok 3";
fi
cd /tmp
umount /tmp/msdosfstest/
mdconfig -d -u 10
rmdir /tmp/msdosfstest/