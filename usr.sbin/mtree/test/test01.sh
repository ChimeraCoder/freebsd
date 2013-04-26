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


ln -s "xx this=is=wrong" ${TMP}/mr/foo
mtree -c -p ${TMP}/mr > ${TMP}/_

if mtree -U -r -p ${TMP}/mt < ${TMP}/_ > /dev/null 2>&1 ; then
	true
else
	echo "ERROR Mtree failed on symlink with space char" 1>&2
	rm -rf ${TMP}
	exit 1
fi

x=x`(cd ${TMP}/mr ; ls -l foo 2>&1) || true`
y=x`(cd ${TMP}/mt ; ls -l foo 2>&1) || true`

if [ "$x" != "$y" ] ; then
	echo "ERROR Recreation of spaced symlink failed" 1>&2
	rm -rf ${TMP}
	exit 1
fi

rm -rf ${TMP}
exit 0