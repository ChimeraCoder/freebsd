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
# clean up the mess that NT makes of my source tree
#
if [ -f makefile -a ! -f Makefile ]; then
	/bin/mv makefile Makefile
fi
chmod +x Configure util/*
echo cleaning
/bin/rm -f `find . -name '*.$$$' -print` 2>/dev/null >/dev/null
echo 'removing those damn ^M'
perl -pi -e 's/\015//' `find . -type 'f' -print |grep -v '.obj$' |grep -v '.der$' |grep -v '.gz'`
make -f Makefile links