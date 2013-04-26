
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
# If you change this file, please also look at files which source this one:
# elf32ltsmip.sh
. ${srcdir}/emulparams/elf32bmip.sh
OUTPUT_FORMAT="elf32-tradbigmips"
BIG_OUTPUT_FORMAT="elf32-tradbigmips"
LITTLE_OUTPUT_FORMAT="elf32-tradlittlemips"
unset DATA_ADDR
SHLIB_TEXT_START_ADDR=0
ENTRY=__start