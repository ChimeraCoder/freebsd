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
# Test for POSIX.2 options for grep
#
# grep -E -f pattern_file file
# grep -F -f pattern_file file
# grep -G -f pattern_file file
#
: ${srcdir=.}

failures=0

cat <<EOF >patfile
radar
MILES
GNU
EOF

# match
echo "miles" |  ${GREP} -i -E -f patfile > /dev/null 2>&1
if test $? -ne 0 ; then
        echo "File_pattern: Wrong status code, test \#1 failed"
        failures=1
fi

# match 
echo "GNU" | ${GREP} -G -f patfile  > /dev/null 2>&1
if test $? -ne 0 ; then
        echo "File_pattern: Wrong status code, test \#2 failed"
        failures=1
fi

# checking for no match
echo "ridar" | ${GREP} -F -f patfile > /dev/null 2>&1
if test $? -ne 1 ; then
	echo "File_pattern: Wrong status code, test \#3 failed"
	failures=1
fi

cat <<EOF >patfile

EOF
# empty pattern : every match
echo "abbcd" | ${GREP} -F -f patfile > /dev/null 2>&1
if test $? -ne 0 ; then
	echo "File_pattern: Wrong status code, test \#4 failed"
	failures=1
fi

cp /dev/null patfile

# null pattern : no match
echo "abbcd" | ${GREP} -F -f patfile > /dev/null 2>&1
if test $? -ne 1 ; then
	echo "File_pattern: Wrong status code, test \#5 failed"
	failures=1
fi

exit $failures