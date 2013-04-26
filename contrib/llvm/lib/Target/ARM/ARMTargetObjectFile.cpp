
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

#include "ARMTargetObjectFile.h"
#include "ARMSubtarget.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/ELF.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;
using namespace dwarf;

//===----------------------------------------------------------------------===//
//                               ELF Target
//===----------------------------------------------------------------------===//

void ARMElfTargetObjectFile::Initialize(MCContext &Ctx,
                                        const TargetMachine &TM) {
  bool isAAPCS_ABI = TM.getSubtarget<ARMSubtarget>().isAAPCS_ABI();
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
  InitializeELF(isAAPCS_ABI);

  if (isAAPCS_ABI) {
    LSDASection = NULL;
  }

  AttributesSection =
    getContext().getELFSection(".ARM.attributes",
                               ELF::SHT_ARM_ATTRIBUTES,
                               0,
                               SectionKind::getMetadata());
}

const MCExpr *ARMElfTargetObjectFile::
getTTypeGlobalReference(const GlobalValue *GV, Mangler *Mang,
                        MachineModuleInfo *MMI, unsigned Encoding,
                        MCStreamer &Streamer) const {
  assert(Encoding == DW_EH_PE_absptr && "Can handle absptr encoding only");

  return MCSymbolRefExpr::Create(Mang->getSymbol(GV),
                                 MCSymbolRefExpr::VK_ARM_TARGET2,
                                 getContext());
}