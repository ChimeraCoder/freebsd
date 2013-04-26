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
# test to see if msdosfs_conv.c rev 1.45[1] works properly.
mkdir /tmp/msdosfstest
mdconfig -a -t swap -s 128m -u 10
bsdlabel -w md10 auto
newfs_msdos -F 16 -b 8192 /dev/md10a
mount_msdosfs -L uk_UA.KOI8-U -D CP866 -l /dev/md10a /tmp/msdosfstest
# The comment is UTF-8, the actual command uses the KOI8-U representation.
# mkdir /tmp/msdosfstest/і (CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I)
mkdir /tmp/msdosfstest/$'\246'
if [ $? -eq 0 ]; then
	echo "ok 4 (pass stage 1/3)"
	cd /tmp/msdosfstest/$'\246'
	if [ $? -eq 0 ]; then
		echo "ok 4 (pass stage 2/3)"
		cd /tmp
		umount /tmp/msdosfstest
		mount_msdosfs -L uk_UA.KOI8-U -D CP866 -s /dev/md10a /tmp/msdosfstest
		cd /tmp/msdosfstest/_~1
		if [ $? -eq 0 ]; then
			echo "ok 4 (pass stage 3/3)"
		else
			echo "not ok 4"
		fi
	else
		echo "not ok 4"
	fi
else
	echo "not ok 4"
fi
cd /tmp
umount /tmp/msdosfstest
mdconfig -d -u 10
rmdir /tmp/msdosfstest