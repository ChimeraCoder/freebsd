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

# Various regression tests to test the -I flag to the 'update' command.

WORKDIR=work

usage()
{
	echo "Usage: ignore.sh [-w workdir]"
	exit 1
}

# Allow the user to specify an alternate work directory.
while getopts "w:" option; do
	case $option in
		w)
			WORKDIR=$OPTARG
			;;
		*)
			echo
			usage
			;;
	esac
done
shift $((OPTIND - 1))
if [ $# -ne 0 ]; then
	usage
fi

CONFLICTS=$WORKDIR/conflicts
OLD=$WORKDIR/old
NEW=$WORKDIR/current
TEST=$WORKDIR/test

# These tests deal with ignoring certain patterns of files.  We run the
# test multiple times ignoring different patterns.
build_trees()
{
	local i

	rm -rf $OLD $NEW $TEST $CONFLICTS
	mkdir -p $OLD $NEW $TEST

	for i in $OLD $NEW $TEST; do
		mkdir -p $i/tree
	done

	# tree: Test three different cases (add, modify, remove) that all
	# match the tree/* glob.
	echo "foo" > $NEW/tree/add
	for i in $OLD $TEST; do
		echo "old" > $i/tree/modify
	done
	echo "new" > $NEW/tree/modify
	for i in $OLD $TEST; do
		echo "old" > $i/tree/remove
	done

	# rmdir: Remove a whole tree.
	for i in $OLD $TEST; do
		mkdir $i/rmdir
		echo "foo" > $i/rmdir/file
	done
}

# $1 - relative path to file that should be missing from TEST
missing()
{
	if [ -e $TEST/$1 -o -L $TEST/$1 ]; then
		echo "File $1 should be missing"
	fi
}

# $1 - relative path to file that should be present in TEST
present()
{
	if ! [ -e $TEST/$1 -o -L $TEST/$1 ]; then
		echo "File $1 should be present"
	fi
}

# $1 - relative path to file that should be a directory in TEST
dir()
{
	if ! [ -d $TEST/$1 ]; then
		echo "File $1 should be a directory"
	fi
}

# $1 - relative path to regular file that should be present in TEST
# $2 - optional string that should match file contents
# $3 - optional MD5 of the flie contents, overrides $2 if present
file()
{
	local contents sum

	if ! [ -f $TEST/$1 ]; then
		echo "File $1 should be a regular file"
	elif [ $# -eq 2 ]; then
		contents=`cat $TEST/$1`
		if [ "$contents" != "$2" ]; then
			echo "File $1 has wrong contents"
		fi
	elif [ $# -eq 3 ]; then
		sum=`md5 -q $TEST/$1`
		if [ "$sum" != "$3" ]; then
			echo "File $1 has wrong contents"
		fi
	fi
}

# $1 - relative path to a regular file that should have a conflict
# $2 - optional MD5 of the conflict file contents
conflict()
{
	local sum

	if ! [ -f $CONFLICTS/$1 ]; then
		echo "File $1 missing conflict"
	elif [ $# -gt 1 ]; then
		sum=`md5 -q $CONFLICTS/$1`
		if [ "$sum" != "$2" ]; then
			echo "Conflict $1 has wrong contents"
		fi
	fi
}

# $1 - relative path to a regular file that should not have a conflict
noconflict()
{
	if [ -f $CONFLICTS/$1 ]; then
		echo "File $1 should not have a conflict"
	fi
}

if [ `id -u` -ne 0 ]; then
	echo "must be root"
fi

if [ -r /etc/etcupdate.conf ]; then
	echo "WARNING: /etc/etcupdate.conf settings may break some tests."
fi

# First run the test ignoring no patterns.

build_trees

etcupdate -r -d $WORKDIR -D $TEST > $WORKDIR/test.out

cat > $WORKDIR/correct.out <<EOF
  D /rmdir/file
  D /tree/remove
  D /rmdir
  U /tree/modify
  A /tree/add
EOF

echo "Differences for regular:"
diff -u -L "correct" $WORKDIR/correct.out -L "test" $WORKDIR/test.out

missing /tree/remove
file /tree/modify "new"
file /tree/add "foo"
missing /rmdir/file
missing /rmdir

# Now test with -I '/tree/*'.  This should preserve the /tree files.

build_trees

etcupdate -r -I '/tree/*' -d $WORKDIR -D $TEST > $WORKDIR/test1.out

cat > $WORKDIR/correct1.out <<EOF
  D /rmdir/file
  D /rmdir
EOF

echo "Differences for -I '/tree/*':"
diff -u -L "correct" $WORKDIR/correct1.out -L "test" $WORKDIR/test1.out

file /tree/remove "old"
file /tree/modify "old"
missing /tree/add
missing /rmdir/file
missing /rmdir

# Now test with two patterns.  This should preserve everything.

build_trees

etcupdate -r -I '/tree/*' -I '/rmdir*' -d $WORKDIR -D $TEST > \
    $WORKDIR/test2.out

cat > $WORKDIR/correct2.out <<EOF
EOF

echo "Differences for -I '/tree/*' -I '/rmdir*':"

diff -u -L "correct" $WORKDIR/correct2.out -L "test" $WORKDIR/test2.out

file /tree/remove "old"
file /tree/modify "old"
missing /tree/add
file /rmdir/file "foo"

# Now test with a pattern that should cause a warning on /rmdir by
# only ignoring the files under that directory.  Note that this also
# tests putting two patterns into a single -I argument.

build_trees

etcupdate -r -I '/tree/* /rmdir/*' -d $WORKDIR -D $TEST > \
    $WORKDIR/test3.out

cat > $WORKDIR/correct3.out <<EOF
Warnings:
  Non-empty directory remains: /rmdir
EOF

echo "Differences for -I '/tree/* /rmdir/*':"

diff -u -L "correct" $WORKDIR/correct3.out -L "test" $WORKDIR/test3.out

file /tree/remove "old"
file /tree/modify "old"
missing /tree/add
file /rmdir/file "foo"
dir /rmdir