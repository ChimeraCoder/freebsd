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

if test $# != 1 ; then
	echo '? expected a single filename'
	exit 1
fi

COL=col$$
INPUT=input$$
UPPER=upper$$
SCRIPT=script$$
RESULT=result$$
rm -f $UPPER $SCRIPT $RESULT
trap "rm -f $COL.* $INPUT $UPPER $SCRIPT $RESULT" 0 1 2 5 15
fgrep -v \# $1 | \
sed	-e 's/[	][	]*/	/g' >$INPUT

for F in 1 2 3 4
do
sed	-e 's/\./	/g' $INPUT | \
cut	-f $F > $COL.$F
done
for F in 2 4
do
	tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ <$COL.$F >$UPPER
	mv $UPPER $COL.$F 
done
paste $COL.* | \
sed	-e 's/^/s\/\\</' \
	-e 's/$/\//' >$UPPER

echo "# Do the TH lines" >>$RESULT
sed	-e 's/\//\/TH /' \
	-e 's/	/ /' \
	-e 's/	/ ""\/TH /' \
	-e 's/	/ /' \
	-e 's/\/$/ ""\//' \
	$UPPER >>$RESULT

echo "# Do the embedded references" >>$RESULT
sed	-e 's/</<fB/' \
	-e 's/	/\\\\fR(/' \
	-e 's/	/)\/fB/' \
	-e 's/	/\\\\fR(/' \
	-e 's/\/$/)\//' \
	$UPPER >>$RESULT

echo "# Do the \fBxxx\fR references in the .NAME section" >>$RESULT
sed	-e 's/\\</^\\\\fB/' \
	-e 's/	[^	]*	/\\\\f[RP] -\/\\\\fB/' \
	-e 's/	.*$/\\\\fR -\//' \
	$UPPER >>$RESULT

# Finally, send the result to standard output
cat $RESULT