
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
if [ -z "${SH}" ]; then
	echo '${SH} is not set, please correct and re-run.'
	exit 1
fi
export SH=${SH}

COUNTER=1

do_test() {
	c=${COUNTER}
	COUNTER=$((COUNTER+1))
	${SH} $1 > tmp.stdout 2> tmp.stderr
	if [ $? -ne $2 ]; then
		echo "not ok ${c} - ${1} # wrong exit status"
		rm tmp.stdout tmp.stderr
		return
	fi
	for i in stdout stderr; do
		if [ -f ${1}.${i} ]; then
			if ! cmp -s tmp.${i} ${1}.${i}; then
				echo "not ok ${c} - ${1} # wrong output on ${i}"
				rm tmp.stdout tmp.stderr
				return
			fi
		elif [ -s tmp.${i} ]; then
			echo "not ok ${c} - ${1} # wrong output on ${i}"
			rm tmp.stdout tmp.stderr
			return
		fi
	done
	echo "ok ${c} - ${1}"
	rm tmp.stdout tmp.stderr
}

TESTS=$(find -Es . -regex ".*\.[0-9]+")
printf "1..%d\n" $(echo ${TESTS} | wc -w)

for i in ${TESTS} ; do
	do_test ${i} ${i##*.}
done