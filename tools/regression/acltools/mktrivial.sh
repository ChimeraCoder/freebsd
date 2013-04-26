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

# This shell script generates an input file for the "run" script, used
# to verify generation of trivial ACLs.

echo "$ touch f"
touch f

for s in `jot 7 0 7`; do
	for u in `jot 7 0 7`; do
		for g in `jot 7 0 7`; do
			for o in `jot 7 0 7`; do
				echo "$ chmod 0$s$u$g$o f"
				chmod "0$s$u$g$o" f
				echo "$ ls -l f | cut -d' ' -f1"
				ls -l f | cut -d' ' -f1 | sed 's/^/> /'
				echo "$ getfacl -q f"
				getfacl -q f | sed 's/^/> /'
			done
		done
	done
done

echo "$ rm f"
rm f