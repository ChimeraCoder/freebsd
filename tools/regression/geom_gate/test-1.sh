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
work="/dev/md${us}"
src="/dev/md`expr $us + 1`"
conf=`mktemp /tmp/$base.XXXXXX` || exit 1

mdconfig -a -t malloc -s 1M -u $us || exit 1
mdconfig -a -t malloc -s 1M -u `expr $us + 1` || exit 1
dd if=/dev/random of=$work bs=1m count=1 >/dev/null 2>&1
dd if=/dev/random of=$src bs=1m count=1 >/dev/null 2>&1
sum=`cat $src | md5 -q`

echo "127.0.0.1 RW $work" > $conf
ggated $conf
ggatec create -u $us 127.0.0.1 $work

dd if=${src} of=/dev/ggate${us} bs=1m count=1 >/dev/null 2>&1

if [ `cat $work | md5 -q` != $sum ]; then
	echo "FAIL"
else
	if [ `cat /dev/ggate${us} | md5 -q` != $sum ]; then
		echo "FAIL"
	else
		echo "PASS"
	fi
fi

ggatec destroy -u $us
mdconfig -d -u $us
mdconfig -d -u `expr $us + 1`
pkill ggated $conf
rm -f $conf