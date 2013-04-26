
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

#include "AArch64Subtarget.h"
#include "AArch64RegisterInfo.h"
#include "MCTargetDesc/AArch64MCTargetDesc.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/ADT/SmallVector.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "AArch64GenSubtargetInfo.inc"

using namespace llvm;

AArch64Subtarget::AArch64Subtarget(StringRef TT, StringRef CPU, StringRef FS)
  : AArch64GenSubtargetInfo(TT, CPU, FS)
  , HasNEON(true)
  , HasCrypto(true)
  , TargetTriple(TT) {

  ParseSubtargetFeatures(CPU, FS);
}

bool AArch64Subtarget::GVIsIndirectSymbol(const GlobalValue *GV,
                                          Reloc::Model RelocM) const {
  if (RelocM == Reloc::Static)
    return false;

  return !GV->hasLocalLinkage() && !GV->hasHiddenVisibility();
}