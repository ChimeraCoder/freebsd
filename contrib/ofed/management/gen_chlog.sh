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

usage()
{
	echo "Usage: $0 [--spec] <target>"
	exit 2
}

test -z "$1" && usage

if [ "$1" = "--spec" ] ; then
	spec_format=1
	shift
	test -z "$1" && usage
fi

TARGET=$1

GIT_DIR=`git rev-parse --git-dir 2>/dev/null`

test -z "$GIT_DIR" && usage


export GIT_DIR
export GIT_PAGER=""
export PAGER=""


mkchlog()
{
	target=$1
	format=$2

	prev_tag=""

	for tag in `git tag -l ${target}-'*'` ; do
		obj=`git cat-file tag $tag | awk '/^object /{print $2}'`
		base=`git merge-base $obj HEAD`
		if [ -z "$base" -o "$base" != $obj ] ; then
			continue
		fi
		all_vers="$prev_tag$tag $all_vers"
		prev_tag=$tag..
	done

	if [ -z "$prev_tag" ] ; then
		all_vers=HEAD
	else
		all_vers="${prev_tag}HEAD $all_vers"
	fi

	for ver in $all_vers ; do
		log_out=`git log $ver -- $target`
		if [ -z "$log_out" ] ; then
			continue
		fi
		ver_name=`echo $ver | sed -e 's/^.*\.\.//'`
		echo ""
		echo "** Version: $ver_name"
		echo ""
		git log --no-merges "${format}" $ver -- $target
		prev_t=$tag..
	done
}


if [ -z "$spec_format" ] ; then
	mkchlog $TARGET --pretty=format:"%ad %an%n%H%n%n* %s%n" \
		| sed -e 's/^\* /\t* /'
else
	echo "%changelog"
	mkchlog $TARGET --pretty=format:"- %ad %an: %s"
	echo ""
fi