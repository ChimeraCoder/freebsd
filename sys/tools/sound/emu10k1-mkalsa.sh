
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
GREP=${GREP:-grep}
CC=${CC:-cc}
AWK=${AWK:-awk}
MV=${MV:=mv}
RM=${RM:=rm}
IN=$1
OUT=$2

trap "${RM} -f $OUT.tmp" EXIT

$GREP -v '#include' $IN | \
$CC -E -D__KERNEL__ -dM -  | \
$AWK -F"[     (]" '
/define/  {
	print "#ifndef " $2;
	print;
	print "#endif";
}' > $OUT.tmp
${MV} -f $OUT.tmp $OUT