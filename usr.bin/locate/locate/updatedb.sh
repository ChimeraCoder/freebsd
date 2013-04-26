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

if [ "$(id -u)" = "0" ]; then
	echo ">>> WARNING" 1>&2
	echo ">>> Executing updatedb as root.  This WILL reveal all filenames" 1>&2
	echo ">>> on your machine to all login users, which is a security risk." 1>&2
fi
: ${LOCATE_CONFIG="/etc/locate.rc"}
if [ -f "$LOCATE_CONFIG" -a -r "$LOCATE_CONFIG" ]; then
       . $LOCATE_CONFIG
fi

# The directory containing locate subprograms
: ${LIBEXECDIR:=/usr/libexec}; export LIBEXECDIR
: ${TMPDIR:=/tmp}; export TMPDIR
if ! TMPDIR=`mktemp -d $TMPDIR/locateXXXXXXXXXX`; then
	exit 1
fi

PATH=$LIBEXECDIR:/bin:/usr/bin:$PATH; export PATH


: ${mklocatedb:=locate.mklocatedb}	 # make locate database program
: ${FCODES:=/var/db/locate.database}	 # the database
: ${SEARCHPATHS="/"}		# directories to be put in the database
: ${PRUNEPATHS="/tmp /usr/tmp /var/tmp /var/db/portsnap"} # unwanted directories
: ${PRUNEDIRS=".zfs"}	# unwanted directories, in any parent
: ${FILESYSTEMS="$(lsvfs | tail -n +3 | \
	egrep -vw "loopback|network|synthetic|read-only|0" | \
	cut -d " " -f1)"}		# allowed filesystems
: ${find:=find}

if [ -z "$SEARCHPATHS" ]; then
	echo "$0: empty variable SEARCHPATHS" >&2; exit 1
fi
if [ -z "$FILESYSTEMS" ]; then
	echo "$0: empty variable FILESYSTEMS" >&2; exit 1
fi

# Make a list a paths to exclude in the locate run
excludes="! (" or=""
for fstype in $FILESYSTEMS
do
       excludes="$excludes $or -fstype $fstype"
       or="-or"
done
excludes="$excludes ) -prune"

if [ -n "$PRUNEPATHS" ]; then
	for path in $PRUNEPATHS; do 
		excludes="$excludes -or -path $path -prune"
	done
fi

if [ -n "$PRUNEDIRS" ]; then
	for dir in $PRUNEDIRS; do
		excludes="$excludes -or -name $dir -type d -prune"
	done
fi

tmp=$TMPDIR/_updatedb$$
trap 'rm -f $tmp; rmdir $TMPDIR' 0 1 2 3 5 10 15
		
# search locally
if $find -s $SEARCHPATHS $excludes -or -print 2>/dev/null |
        $mklocatedb -presort > $tmp
then
	if [ -n "$($find $tmp -size -257c -print)" ]; then
		echo "updatedb: locate database $tmp is empty" >&2
		exit 1
	else
		cat $tmp > $FCODES		# should be cp?
	fi
fi