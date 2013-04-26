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

: ${TMPDIR:=/tmp}; export TMPDIR
test -d "$TMPDIR" || TMPDIR=/tmp
if ! TMPDIR=`mktemp -d $TMPDIR/mklocateXXXXXXXXXX`; then
	exit 1
fi


# utilities to built locate database
: ${bigram:=locate.bigram}
: ${code:=locate.code}
: ${sort:=sort}


sortopt="-u -T $TMPDIR"
sortcmd=$sort


bigrams=$TMPDIR/_mklocatedb$$.bigrams
filelist=$TMPDIR/_mklocatedb$$.list

trap 'rm -f $bigrams $filelist; rmdir $TMPDIR' 0 1 2 3 5 10 15


# Input already sorted
if [ X"$1" = "X-presort" ]; then
    shift; 

    # create an empty file
    true > $bigrams
    
    # Locate database bootstrapping
    # 1. first build a temp database without bigram compression
    # 2. create the bigram from the temp database
    # 3. create the real locate database with bigram compression.
    #
    # This scheme avoid large temporary files in /tmp

    $code $bigrams > $filelist || exit 1
    locate -d $filelist / | $bigram | $sort -nr | head -128 |
    awk '{if (/^[ 	]*[0-9]+[ 	]+..$/) {printf("%s",$2)} else {exit 1}}' > $bigrams || exit 1
    locate -d $filelist / | $code $bigrams || exit 1
    exit 	

else
    if $sortcmd $sortopt > $filelist; then
        $bigram < $filelist | $sort -nr | 
	awk '{if (/^[ 	]*[0-9]+[ 	]+..$/) {printf("%s",$2)} else {exit 1}}' > $bigrams || exit 1
        $code $bigrams < $filelist || exit 1
    else
        echo "`basename $0`: cannot build locate database" >&2
        exit 1
    fi
fi