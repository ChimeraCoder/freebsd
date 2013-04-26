
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
#include "MCTargetDesc/MipsReginfo.h"
#include "MipsSubtarget.h"
#include "MipsTargetObjectFile.h"
#include "llvm/MC/MCStreamer.h"

using namespace llvm;

// Integrated assembler version
void
MipsReginfo::emitMipsReginfoSectionCG(MCStreamer &OS,
    const TargetLoweringObjectFile &TLOF,
    const MipsSubtarget &MST) const
{

  if (OS.hasRawTextSupport())
    return;

  const MipsTargetObjectFile &TLOFELF =
      static_cast<const MipsTargetObjectFile &>(TLOF);
  OS.SwitchSection(TLOFELF.getReginfoSection());

  // .reginfo
  if (MST.isABI_O32()) {
    OS.EmitIntValue(0, 4); // ri_gprmask
    OS.EmitIntValue(0, 4); // ri_cpr[0]mask
    OS.EmitIntValue(0, 4); // ri_cpr[1]mask
    OS.EmitIntValue(0, 4); // ri_cpr[2]mask
    OS.EmitIntValue(0, 4); // ri_cpr[3]mask
    OS.EmitIntValue(0, 4); // ri_gp_value
  }
  // .MIPS.options
  else if (MST.isABI_N64()) {
    OS.EmitIntValue(1, 1); // kind
    OS.EmitIntValue(40, 1); // size
    OS.EmitIntValue(0, 2); // section
    OS.EmitIntValue(0, 4); // info
    OS.EmitIntValue(0, 4); // ri_gprmask
    OS.EmitIntValue(0, 4); // pad
    OS.EmitIntValue(0, 4); // ri_cpr[0]mask
    OS.EmitIntValue(0, 4); // ri_cpr[1]mask
    OS.EmitIntValue(0, 4); // ri_cpr[2]mask
    OS.EmitIntValue(0, 4); // ri_cpr[3]mask
    OS.EmitIntValue(0, 8); // ri_gp_value
  }
  else llvm_unreachable("Unsupported abi for reginfo");
}