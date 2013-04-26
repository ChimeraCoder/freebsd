
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

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Target/TargetOptions.h"
using namespace llvm;

/// DisableFramePointerElim - This returns true if frame pointer elimination
/// optimization should be disabled for the given machine function.
bool TargetOptions::DisableFramePointerElim(const MachineFunction &MF) const {
  // Check to see if we should eliminate non-leaf frame pointers and then
  // check to see if we should eliminate all frame pointers.
  if (NoFramePointerElimNonLeaf && !NoFramePointerElim) {
    const MachineFrameInfo *MFI = MF.getFrameInfo();
    return MFI->hasCalls();
  }

  return NoFramePointerElim;
}

/// LessPreciseFPMAD - This flag return true when -enable-fp-mad option
/// is specified on the command line.  When this flag is off(default), the
/// code generator is not allowed to generate mad (multiply add) if the
/// result is "less precise" than doing those operations individually.
bool TargetOptions::LessPreciseFPMAD() const {
  return UnsafeFPMath || LessPreciseFPMADOption;
}

/// HonorSignDependentRoundingFPMath - Return true if the codegen must assume
/// that the rounding mode of the FPU can change from its default.
bool TargetOptions::HonorSignDependentRoundingFPMath() const {
  return !UnsafeFPMath && HonorSignDependentRoundingFPMathOption;
}

/// getTrapFunctionName - If this returns a non-empty string, this means isel
/// should lower Intrinsic::trap to a call to the specified function name
/// instead of an ISD::TRAP node.
StringRef TargetOptions::getTrapFunctionName() const {
  return TrapFuncName;
}