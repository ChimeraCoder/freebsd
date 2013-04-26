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

: ${ticdir=@TERMINFO@}
: ${xterm_new=@WHICH_XTERM@}

# If we're not installing into /usr/share/, we'll have to adjust the location
# of the tabset files in terminfo.src (which are in a parallel directory).
TABSET=`echo $ticdir | sed -e 's%/terminfo$%/tabset%'`
if test "x$TABSET" != "x/usr/share/tabset" ; then
cat <<EOF
s%/usr/share/tabset%$TABSET%g
EOF
fi

if test "$xterm_new" != "xterm-new" ; then
cat <<EOF
/^# This is xterm for ncurses/,/^$/{
	s/use=xterm-new,/use=$WHICH_XTERM,/
}
EOF
fi