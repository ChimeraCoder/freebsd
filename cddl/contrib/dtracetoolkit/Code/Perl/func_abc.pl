#!./perl -w
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

sub func_c {
    print "Function C\n";
    sleep 1;
}

sub func_b {
    print "Function B\n";
    sleep 1;
    func_c();
}

sub func_a {
    print "Function A\n";
    sleep 1;
    func_b();
}

func_a();