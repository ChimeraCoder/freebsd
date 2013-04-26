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
mkdir -p ${TMP}

K=uid,uname,gid,gname,flags,md5digest,size,ripemd160digest,sha1digest,sha256digest,cksum

rm -rf _FOO
mkdir _FOO
touch _FOO/_uid
touch _FOO/_size
touch _FOO/zztype

touch _FOO/_bar
mtree -c -K $K -p .. > ${TMP}/_r
mtree -c -K $K -p .. > ${TMP}/_r2
rm -rf _FOO/_bar 

rm -rf _FOO/zztype
mkdir _FOO/zztype

date > _FOO/_size

chown nobody _FOO/_uid

touch _FOO/_foo
mtree -c -K $K -p .. > ${TMP}/_t

rm -fr _FOO

if mtree -f ${TMP}/_r -f ${TMP}/_r2 ; then
	true
else
	echo "ERROR Compare identical failed" 1>&2
	exit 1
fi
	
if mtree -f ${TMP}/_r -f ${TMP}/_t > ${TMP}/_ ; then
	echo "ERROR Compare different succeeded" 1>&2
	exit 1
fi

if [ `wc -l  < ${TMP}/_` -ne 10 ] ; then
	echo "ERROR wrong number of lines: `wc -l  ${TMP}/_`" 1>&2
	exit 1
fi
	
exit 0