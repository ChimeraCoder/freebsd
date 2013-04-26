
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

#include "llvm/Support/Watchdog.h"
#include "llvm/Config/config.h"

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Watchdog.inc"
#endif
#ifdef LLVM_ON_WIN32
#include "Windows/Watchdog.inc"
#endif