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


mkdir ${TMP}/mt/foo
mkdir ${TMP}/mr/\*
mtree -c -p ${TMP}/mr | mtree -U -r -p ${TMP}/mt > /dev/null 2>&1
if [ -d ${TMP}/mt/foo ] ; then
	echo "ERROR Mtree create fell for filename with '*' char" 1>&2
	rm -rf ${TMP}
	exit 1
fi
rmdir ${TMP}/mr/\*

mkdir -p ${TMP}/mt/foo
mkdir ${TMP}/mr/\[f\]oo
mtree -c -p ${TMP}/mr | mtree -U -r -p ${TMP}/mt > /dev/null 2>&1
if [ -d ${TMP}/mt/foo ] ; then
	echo "ERROR Mtree create fell for filename with '[' char" 1>&2
	rm -rf ${TMP}
	exit 1
fi
rmdir ${TMP}/mr/\[f\]oo

mkdir -p ${TMP}/mt/foo
mkdir ${TMP}/mr/\?oo
mtree -c -p ${TMP}/mr | mtree -U -r -p ${TMP}/mt > /dev/null 2>&1
if [ -d ${TMP}/mt/foo ] ; then
	echo "ERROR Mtree create fell for filename with '?' char" 1>&2
	rm -rf ${TMP}
	exit 1
fi
rmdir ${TMP}/mr/\?oo

mkdir ${TMP}/mr/\#
mtree -c -p ${TMP}/mr > ${TMP}/_
if mtree -U -r -p ${TMP}/mt < ${TMP}/_ > /dev/null 2>&1 ; then
	true
else
	echo "ERROR Mtree create fell for filename with '#' char" 1>&2
	rm -rf ${TMP}
	exit 1
fi
	
if [ ! -d ${TMP}/mt/\# ] ; then
	echo "ERROR Mtree update failed to create name with '#' char" 1>&2
	rm -rf ${TMP}
	exit 1
fi
rmdir ${TMP}/mr/\#

rm -rf ${TMP}
exit 0