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
set -e
MD=`mdconfig -a -t malloc -s 4m -x 63 -y 16`
if [ ! -c /dev/${MD} ] ; then
	echo "MD device $MD did not materialize" 1>&2
	exit 2
fi
trap "mdconfig -d -u ${MD}" EXIT INT TERM

# Create an empty bootcode file to isolate our checksum from any changes
# which might happen to the boot code file.
dd if=/dev/zero of=tmp count=1 > /dev/null 2>&1
./fdisk -b tmp -I $MD > /dev/null 2>&1
rm tmp

c=`dd if=/dev/${MD} count=1 2>/dev/null | md5`
if [ $c != ea4277fcccb6a927a1a497a6b15bfb8c ] ; then
	echo "FAILED: 'fdisk -I' gives bad checksum ($c)" 1>&2
	exit 1
fi
echo "PASSED: fdisk -I"
c=`./fdisk $MD | md5`
if [ $c != 4b126d7ac4c6b2af7ef27ede8ef102ec ] ; then
	echo "FAILED: 'fdisk' gives bad checksum ($c)" 1>&2
	exit 1
fi
echo "PASSED: fdisk"
exit 0