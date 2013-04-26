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
### quick sanity test for the binutils.
###
### This file was written and is maintained by K. Richard Pixley,
### rich@cygnus.com.
### fail on errors
set -e

### first arg is directory in which binaries to be tested reside.
case "$1" in
"") BIN=. ;;
*)  BIN="$1" ;;
esac

### size
for i in size objdump nm ar strip ranlib ; do
	${BIN}/size ${BIN}/$i > /dev/null
done

### objdump
for i in size objdump nm ar strip ranlib ; do
	${BIN}/objdump -ahifdrtxsl ${BIN}/$i > /dev/null
done

### nm
for i in size objdump nm ar strip ranlib ; do
	${BIN}/nm ${BIN}/$i > /dev/null
done

### strip
TMPDIR=./binutils-$$
mkdir ${TMPDIR}

cp ${BIN}/strip ${TMPDIR}/strip

for i in size objdump nm ar ranlib ; do
	cp ${BIN}/$i ${TMPDIR}/$i
	${BIN}/strip ${TMPDIR}/$i
	cp ${BIN}/$i ${TMPDIR}/$i
	${TMPDIR}/strip ${TMPDIR}/$i
done

### ar

### ranlib

rm -rf ${TMPDIR}

exit 0