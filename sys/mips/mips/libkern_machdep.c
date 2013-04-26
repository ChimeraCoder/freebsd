
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Include libkern support routines for 64-bit operations when building o32
 * kernels.
 */
#if defined(__mips_o32)
#include <libkern/divdi3.c>
#include <libkern/moddi3.c>
#include <libkern/qdivrem.c>
#include <libkern/udivdi3.c>
#include <libkern/umoddi3.c>
#endif