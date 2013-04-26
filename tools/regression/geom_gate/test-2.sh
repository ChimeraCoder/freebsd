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
base=`basename $0`
us=45
work=`mktemp /tmp/$base.XXXXXX` || exit 1
src=`mktemp /tmp/$base.XXXXXX` || exit 1

dd if=/dev/random of=$work bs=1m count=1 >/dev/null 2>&1
dd if=/dev/random of=$src bs=1m count=1 >/dev/null 2>&1
sum=`md5 -q $src`

ggatel create -u $us $work

dd if=${src} of=/dev/ggate${us} bs=1m count=1 >/dev/null 2>&1

if [ `md5 -q $work` != $sum ]; then
	echo "FAIL"
else
	if [ `cat /dev/ggate${us} | md5 -q` != $sum ]; then
		echo "FAIL"
	else
		echo "PASS"
	fi
fi

ggatel destroy -u $us
rm -f $work $src