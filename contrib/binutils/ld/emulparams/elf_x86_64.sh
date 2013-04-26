
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
# Linux/Solaris modify the default library search path to first include
# a 64-bit specific directory.case "$target" in
  x86_64*-linux*|i[3-7]86-*-linux-*)
    case "$EMULATION_NAME" in
      *64*) LIBPATH_SUFFIX=64 ;;
    esac
    ;;
  *-*-solaris2*) 
    LIBPATH_SUFFIX=/amd64
    ELF_INTERPRETER_NAME=\"/lib/amd64/ld.so.1\"
  ;;
esac