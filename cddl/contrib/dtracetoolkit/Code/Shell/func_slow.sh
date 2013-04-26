#!./sh
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

func_c()
{
	echo "Function C"
	i=0
	while [ $i -lt 300 ]
	do
		i=`expr $i + 1`
	done
}

func_b()
{
	echo "Function B"
	i=0
	while [ $i -lt 200 ]
	do
		i=`expr $i + 1`
	done
	func_c
}

func_a()
{
	echo "Function A"
	i=0
	while [ $i -lt 100 ]
	do
		i=`expr $i + 1`
	done
	func_b
}

func_a