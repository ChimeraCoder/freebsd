
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

#include "XCoreSubtarget.h"
#include "XCore.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "XCoreGenSubtargetInfo.inc"

using namespace llvm;

void XCoreSubtarget::anchor() { }

XCoreSubtarget::XCoreSubtarget(const std::string &TT,
                               const std::string &CPU, const std::string &FS)
  : XCoreGenSubtargetInfo(TT, CPU, FS)
{
}