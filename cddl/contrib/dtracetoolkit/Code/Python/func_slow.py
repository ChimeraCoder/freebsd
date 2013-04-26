#!/usr/bin/python
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

def func_c():
	print "Function C"
	i = 0
	while (i < 3000000):
		i = i + 1
		j = i + 1

def func_b():
	print "Function B"
	i = 0
	while (i < 2000000):
		i = i + 1
		j = i + 1
	func_c()

def func_a():
	print "Function A"
	i = 0
	while (i < 1000000):
		i = i + 1
		j = i + 1
	func_b()

func_a()