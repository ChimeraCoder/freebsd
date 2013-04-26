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
#
# $FreeBSD$
#
# Test for 'optional' keyword.
#
TMP=`mktemp -d /tmp/mtree.XXXXXX`
mkdir -p ${TMP}/mr ${TMP}/mr/optional-dir ${TMP}/mr/some-dir
touch ${TMP}/mr/optional-file ${TMP}/mr/some-file

mtree -c -p ${TMP}/mr > ${TMP}/_
rm -rf ${TMP}/mr/optional-file ${TMP}/mr/optional-dir
mtree -p ${TMP}/mr -K optional < ${TMP}/_ > /dev/null

res=$?

if [ $res -ne 0 ] ; then
	echo "ERROR 'optional' keyword failed" 1>&2
	rm -rf ${TMP}
	exit 1
fi

rm -rf ${TMP}
exit 0