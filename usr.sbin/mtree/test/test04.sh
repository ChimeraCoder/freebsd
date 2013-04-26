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

set -e

TMP=/tmp/mtree.$$

rm -rf ${TMP}
mkdir -p ${TMP} ${TMP}/mr ${TMP}/mt

mkdir ${TMP}/mr/a
mkdir ${TMP}/mr/b
mkdir ${TMP}/mt/a
mkdir ${TMP}/mt/b
touch ${TMP}/mt/z

mtree -c -p ${TMP}/mr > ${TMP}/_r
mtree -c -p ${TMP}/mt > ${TMP}/_t

if mtree -f ${TMP}/_r -f ${TMP}/_t > ${TMP}/_ ; then
	echo "ERROR wrong exit on difference" 1>&2
	exit 1
fi

if [ `wc -l < ${TMP}/_` -ne 1 ] ; then
	echo "ERROR spec/spec compare generated wrong output" 1>&2
	rm -rf ${TMP}
	exit 1
fi

if mtree -f ${TMP}/_t -f ${TMP}/_r > ${TMP}/_ ; then
	echo "ERROR wrong exit on difference" 1>&2
	exit 1
fi

if [ `wc -l < ${TMP}/_` -ne 1 ] ; then
	echo "ERROR spec/spec compare generated wrong output" 1>&2
	rm -rf ${TMP}
	exit 1
fi

rm -rf ${TMP}
exit 0