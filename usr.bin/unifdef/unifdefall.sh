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

unifdef="$(dirname "$0")/unifdef"
if [ ! -e "$unifdef" ]
then
	unifdef=unifdef
fi

case "$@" in
"-d "*)	echo DEBUGGING 1>&2
	debug=-d
	shift
esac

tmp=$(mktemp -d "${TMPDIR:-/tmp}/${0##*/}.XXXXXXXXXX") || exit 2
trap 'rm -r "$tmp" || exit 2' EXIT

export LC_ALL=C

# list of all controlling macros
"$unifdef" $debug -s "$@" | sort | uniq >"$tmp/ctrl"
# list of all macro definitions
cpp -dM "$@" | sort | sed 's/^#define //' >"$tmp/hashdefs"
# list of defined macro names
sed 's/[^A-Za-z0-9_].*$//' <"$tmp/hashdefs" >"$tmp/alldef"
# list of undefined and defined controlling macros
comm -23 "$tmp/ctrl" "$tmp/alldef" >"$tmp/undef"
comm -12 "$tmp/ctrl" "$tmp/alldef" >"$tmp/def"
# create a sed script that extracts the controlling macro definitions
# and converts them to unifdef command-line arguments
sed 's|.*|s/^&\\(([^)]*)\\)\\{0,1\\} /-D&=/p|' <"$tmp/def" >"$tmp/script"
# create the final unifdef command
{	echo "$unifdef" $debug -k '\'
	# convert the controlling undefined macros to -U arguments
	sed 's/.*/-U& \\/' <"$tmp/undef"
	# convert the controlling defined macros to quoted -D arguments
	sed -nf "$tmp/script" <"$tmp/hashdefs" |
		sed "s/'/'\\\\''/g;s/.*/'&' \\\\/"
	echo '"$@"'
} >"$tmp/cmd"
case $debug in
-d)	for i in ctrl hashdefs alldef undef def script cmd
	do	echo ==== $i
		cat "$tmp/$i"
	done 1>&2
esac
# run the command we just created
sh "$tmp/cmd" "$@"