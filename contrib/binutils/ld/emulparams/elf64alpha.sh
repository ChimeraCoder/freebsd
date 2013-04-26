
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
# Yes, we want duplicate .plt sections.  The linker chooses the
# appropriate one magically in alpha_after_open.PLT=".plt          ${RELOCATING-0} : SPECIAL { *(.plt) }"
DATA_PLT=yes
TEXT_PLT=yes

# Note that the number is always big-endian, thus we have to 
# reverse the digit string.
NOP=0x0000fe2f1f04ff47		# unop; nop

OTHER_READONLY_SECTIONS="
  .reginfo      ${RELOCATING-0} : { *(.reginfo) }"