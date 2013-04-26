#!./ruby -w
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

def func_c
  print "Function C\n"
  i = 0
  while i < 300000
     i = i + 1
     j = i + 1
  end
end

def func_b
  print "Function B\n"
  i = 0
  while i < 200000
     i = i + 1
     j = i + 1
  end
  func_c
end

def func_a
  print "Function A\n"
  i = 0
  while i < 100000
     i = i + 1
     j = i + 1
  end
  func_b
end

func_a