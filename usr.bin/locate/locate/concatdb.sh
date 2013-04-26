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

# The directory containing locate subprograms
: ${LIBEXECDIR:=/usr/libexec}; export LIBEXECDIR

PATH=$LIBEXECDIR:/bin:/usr/bin:$PATH; export PATH

umask 077			# protect temp files

: ${TMPDIR:=/var/tmp}; export TMPDIR;
test -d "$TMPDIR" || TMPDIR=/var/tmp

# utilities to built locate database
: ${bigram:=locate.bigram}
: ${code:=locate.code}
: ${sort:=sort}
: ${locate:=locate}


case $# in 
        [01]) 	echo 'usage: concatdb databases1 ... databaseN > newdb'
		exit 1
		;;
esac


bigrams=`mktemp ${TMPDIR=/tmp}/_bigrams.XXXXXXXXXX` || exit 1
trap 'rm -f $bigrams' 0 1 2 3 5 10 15

for db 
do
       $locate -d $db /
done | $bigram | $sort -nr | awk 'NR <= 128 { printf $2 }' > $bigrams

for db
do
	$locate -d $db /
done | $code $bigrams