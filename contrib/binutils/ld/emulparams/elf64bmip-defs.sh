
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
# elf32bmipn32-defs.sh use .reginfo, n64 ABI should use .MIPS.options,
# override INITIAL_READONLY_SECTIONS to do this.INITIAL_READONLY_SECTIONS=
if test -z "${CREATE_SHLIB}"; then
  INITIAL_READONLY_SECTIONS=".interp       ${RELOCATING-0} : { *(.interp) }"
fi
INITIAL_READONLY_SECTIONS="${INITIAL_READONLY_SECTIONS}
  .MIPS.options      ${RELOCATING-0} : { *(.MIPS.options) }"