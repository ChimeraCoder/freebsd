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

macro () {

	n=${1}
	shift
	echo -n "#define bus_${n}(r"
	for i
	do
		echo -n ", ${i}"
	done
	echo ") \\"
	echo -n "	bus_space_${n}((r)->r_bustag, (r)->r_bushandle"
	for i
	do
		echo -n ", (${i})"
	done
	echo ")"
}

macro barrier o l f

for w in 1 2 4 8
do
	# macro copy_region_$w so dh do c
	# macro copy_region_stream_$w ?
	# macro peek_$w
	for s in "" stream_
	do
		macro read_$s$w o
		macro read_multi_$s$w o d c
		macro read_region_$s$w o d c
		macro set_multi_$s$w o v c
		macro set_region_$s$w o v c
		macro write_$s$w o v
		macro write_multi_$s$w o d c
		macro write_region_$s$w o d c
	done
done