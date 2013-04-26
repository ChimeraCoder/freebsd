#!/bin/bash
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

tidy_cmd="perltidy -pt=2 -sbt=2 -bt=2 -nsfs -b -t -nola -ce -sbl -nbbc"

argv0=`basename $0`
scripts_dir=`dirname $0`/scripts

if [ "$1" == "-h" ]; then
   echo "$argv0 [-h]"
   echo "   Run perltidy on all perl scripts and modules in the scripts directory"
   exit 1
fi

cd $scripts_dir

for file in *.pl ; do
   echo "tidy : $scripts_dir/$file"
   $tidy_cmd $file
done

for file in *.pm ; do
   echo "tidy : $scripts_dir/$file"
   $tidy_cmd $file
done

exit 0