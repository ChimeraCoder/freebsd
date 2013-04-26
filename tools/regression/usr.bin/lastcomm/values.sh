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
ACCT=acct
RESULTS=acct.`uname -m`.tar.gz
TMPDIR=$$.tmp

run()
{
	/usr/bin/time -l -o $1.time "$@"
}

if [ `whoami` != "root" ]; then
	echo "You need to be root to run this test."
	exit 1
fi

echo Redirecting accounting.

mkdir $TMPDIR
cd $TMPDIR

:>$ACCT
accton $ACCT

echo Running commands. This should not take more than 30s.

# User time
run awk 'BEGIN {for (i = 0; i < 1000000; i++) cos(.3)}' /dev/null
run egrep '(.)(.)(.)(.)(.)(.)(.)(.)(.)\9\8\7\6\5\4\3\2\1' /usr/share/dict/words

# System time
run find /usr/src -name xyzzy

# Elapsed time
run sleep 3

# IO
run dd if=/dev/zero bs=512 count=4096 of=zero 2>/dev/null

# Memory
run diff /usr/share/dict/words /dev/null >/dev/null

# AC_COMM_LEN - 1
ln /usr/bin/true 123456789012345
run ./123456789012345

# AC_COMM_LEN
ln /usr/bin/true 1234567890123456
run ./1234567890123456

# AC_COMM_LEN + 1
ln /usr/bin/true 12345678901234567
run ./12345678901234567

# Flags: core, fork, signal
echo 'main(){volatile int s; fork(); s = *(int *)0;}' >core.c
cc -o core core.c
echo Generating segmentation fault.
./core

echo Turning off redirected accounting.

accton

echo Packing the results.

sa -u $ACCT >sa.u
lastcomm -cesuS -f $ACCT >lastcomm.cesuS
tar -cf - acct sa.u lastcomm.cesuS *.time |
gzip -c |
uuencode $RESULTS >../$RESULTS.uue

echo Cleaning up.
cd ..
rm -rf $TMPDIR

echo "Done!  Please send your reply, enclosing the file $RESULTS.uue"
echo If your system runs with accounting enabled you probably need to run:
echo accton /var/account/acct