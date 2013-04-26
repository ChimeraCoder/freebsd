#! /bin/sh
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

trap 'rm -f $f1 $f2; exit 1' 1 2 3 13 15
echo shazbot > /dev/null
f1=`mktemp /tmp/filemon_test.XXXXXX`
f2=`mktemp /tmp/ed-script.XXXXXX`
> $f1
echo "One line to rule them all" >> $f1
wc -l $f1 > /dev/null
#	ed(1)'s /tmp/ed.* buffer file will be opened RW
echo ',s/$/./g'	> $f2
echo 'wq'	>>$f2
ed -s $f1 < $f2
#echo ",s/$/./\
#w" | ed -s $f1
#rm $f1 $f2
uptime > /dev/null