#!/bin/sh -
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

# only one argument is a special case, just output the name twice
case $# in
	0)
		echo "usage: lorder file ...";
		exit ;;
	1)
		echo $1 $1;
		exit ;;
esac

# temporary files
R=$(mktemp -t _reference_)
S=$(mktemp -t _symbol_)
NM=${NM:-nm}

# remove temporary files on HUP, INT, QUIT, PIPE, TERM
trap "rm -f $R $S $T; exit 1" 1 2 3 13 15

# make sure all the files get into the output
for i in $*; do
	echo $i $i
done

# if the line has " [TDW] " it's a globally defined symbol, put it
# into the symbol file.
#
# if the line has " U " it's a globally undefined symbol, put it into
# the reference file.
${NM} -go $* | sed "
	/ [TDW] / {
		s/:.* [TDW] / /
		w $S
		d
	}
	/ U / {
		s/:.* U / /
		w $R
	}
	d
"

# eliminate references that can be resolved by the same library.
if [ $(expr "$*" : '.*\.a[[:>:]]') -ne 0 ]; then
	sort -u -o $S $S
	sort -u -o $R $R
	T=$(mktemp -t _temp_)
	comm -23 $R $S >$T
	mv $T $R
fi

# sort references and symbols on the second field (the symbol),
# join on that field, and print out the file names.
sort -k 2 -o $R $R
sort -k 2 -o $S $S
join -j 2 -o 1.1 2.1 $R $S
rm -f $R $S