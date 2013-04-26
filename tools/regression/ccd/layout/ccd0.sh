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
# $FreeBSD$
set -e

make a
make b
foo() {
	f="${1}_${2}_${3}_${4}_${5}_${6}"
	echo $f
	sh ccd.sh $1 $2 $3 $4 $5 $6 > _.$f
	if [ -f ref.$f ] ; then
		diff -u -I '$FreeBSD' ref.$f _.$f
	fi
}

foo 128k 128k 128k 128k 0 0 
foo 128k 128k 128k 128k 0 4
foo 128k 128k 128k 128k 4 0 
foo 128k 128k 128k 128k 4 2
foo 128k 128k 128k 128k 4 4

foo 256k 128k 128k 128k 0 0 
foo 256k 128k 128k 128k 0 4
foo 256k 128k 128k 128k 4 0 
foo 256k 128k 128k 128k 4 2
foo 256k 128k 128k 128k 4 4

foo 256k 128k 384k 128k 0 0 
foo 256k 128k 384k 128k 0 4
foo 256k 128k 384k 128k 4 0 
foo 256k 128k 384k 128k 4 2
foo 256k 128k 384k 128k 4 4

foo 256k 128k 384k 128k 16 0 
foo 256k 128k 384k 128k 16 4
foo 256k 128k 384k 128k 16 0 
foo 256k 128k 384k 128k 16 2
foo 256k 128k 384k 128k 16 4