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
#
# Tell them not to be alarmed.
: ${srcdir=.}

failures=0

#
cat <<\EOF

Please, do not be alarmed if some of the tests failed.
Report them to <bug-gnu-utils@gnu.org>,
with the line number, the name of the file,
and grep version number 'grep --version'.
Make sure you have the word grep in the subject.
Thank You.

EOF