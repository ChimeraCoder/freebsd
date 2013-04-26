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
# grep [ -E| -F][ -c| -l| -q ][-insvx] -e pattern_list 
#      [-f pattern_file] ... [file. ..]
# grep [ -E| -F][ -c| -l| -q ][-insvx][-e pattern_list]
#      -f pattern_file ... [file ...]
# grep [ -E| -F][ -c| -l| -q ][-insvx] pattern_list [file...]
#
: ${srcdir=.}

failures=0

# checking for -E extended regex
echo "abababccccccd" | ${GREP} -E -e 'c{3}' > /dev/null 2>&1
if test $? -ne 0 ; then
        echo "Options: Wrong status code, test \#1 failed"
        failures=1
fi

# checking for basic regex
echo "abababccccccd" | ${GREP} -G -e 'c\{3\}' > /dev/null 2>&1
if test $? -ne 0 ; then
        echo "Options: Wrong status code, test \#2 failed"
        failures=1
fi

# checking for fixed string 
echo "abababccccccd" | ${GREP} -F -e 'c\{3\}' > /dev/null 2>&1
if test $? -ne 1 ; then
	echo "Options: Wrong status code, test \#3 failed"
	failures=1
fi

exit $failures