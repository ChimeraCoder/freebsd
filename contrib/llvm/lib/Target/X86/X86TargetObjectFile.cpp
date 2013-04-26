
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

#include "X86TargetObjectFile.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Target/Mangler.h"

using namespace llvm;
using namespace dwarf;

const MCExpr *X86_64MachoTargetObjectFile::
getTTypeGlobalReference(const GlobalValue *GV, Mangler *Mang,
                        MachineModuleInfo *MMI, unsigned Encoding,
                        MCStreamer &Streamer) const {

  // On Darwin/X86-64, we can reference dwarf symbols with foo@GOTPCREL+4, which
  // is an indirect pc-relative reference.
  if (Encoding & (DW_EH_PE_indirect | DW_EH_PE_pcrel)) {
    const MCSymbol *Sym = Mang->getSymbol(GV);
    const MCExpr *Res =
      MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_GOTPCREL, getContext());
    const MCExpr *Four = MCConstantExpr::Create(4, getContext());
    return MCBinaryExpr::CreateAdd(Res, Four, getContext());
  }

  return TargetLoweringObjectFileMachO::
    getTTypeGlobalReference(GV, Mang, MMI, Encoding, Streamer);
}

MCSymbol *X86_64MachoTargetObjectFile::
getCFIPersonalitySymbol(const GlobalValue *GV, Mangler *Mang,
                        MachineModuleInfo *MMI) const {
  return Mang->getSymbol(GV);
}

void
X86LinuxTargetObjectFile::Initialize(MCContext &Ctx, const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
  InitializeELF(TM.Options.UseInitArray);
}