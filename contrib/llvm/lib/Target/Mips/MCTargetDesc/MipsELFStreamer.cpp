
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
#include "MCTargetDesc/MipsELFStreamer.h"
#include "MipsSubtarget.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELF.h"
#include "llvm/MC/MCELFSymbolFlags.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ELF.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

  MCELFStreamer* createMipsELFStreamer(MCContext &Context, MCAsmBackend &TAB,
                                       raw_ostream &OS, MCCodeEmitter *Emitter,
                                       bool RelaxAll, bool NoExecStack) {
    MipsELFStreamer *S = new MipsELFStreamer(Context, TAB, OS, Emitter,
                                             RelaxAll, NoExecStack);
    return S;
  }

  // For llc. Set a group of ELF header flags
  void
  MipsELFStreamer::emitELFHeaderFlagsCG(const MipsSubtarget &Subtarget) {

    if (hasRawTextSupport())
      return;

    // Update e_header flags
    MCAssembler& MCA = getAssembler();
    unsigned EFlags = MCA.getELFHeaderEFlags();

    if (Subtarget.inMips16Mode())
      EFlags |= ELF::EF_MIPS_ARCH_ASE_M16;
    else
      EFlags |= ELF::EF_MIPS_NOREORDER;

    // Architecture
    if (Subtarget.hasMips64r2())
      EFlags |= ELF::EF_MIPS_ARCH_64R2;
    else if (Subtarget.hasMips64())
      EFlags |= ELF::EF_MIPS_ARCH_64;
    else if (Subtarget.hasMips32r2())
      EFlags |= ELF::EF_MIPS_ARCH_32R2;
    else
      EFlags |= ELF::EF_MIPS_ARCH_32;

    if (Subtarget.inMicroMipsMode())
      EFlags |= ELF::EF_MIPS_MICROMIPS;

    // ABI
    if (Subtarget.isABI_O32())
      EFlags |= ELF::EF_MIPS_ABI_O32;

    // Relocation Model
    Reloc::Model RM = Subtarget.getRelocationModel();
    if (RM == Reloc::PIC_ || RM == Reloc::Default)
      EFlags |= ELF::EF_MIPS_PIC;
    else if (RM == Reloc::Static)
      ; // Do nothing for Reloc::Static
    else
      llvm_unreachable("Unsupported relocation model for e_flags");

    MCA.setELFHeaderEFlags(EFlags);
  }

  // For llc. Set a symbol's STO flags
  void
  MipsELFStreamer::emitMipsSTOCG(const MipsSubtarget &Subtarget,
                                 MCSymbol *Sym,
                                 unsigned Val) {

    if (hasRawTextSupport())
      return;

    MCSymbolData &Data = getOrCreateSymbolData(Sym);
    // The "other" values are stored in the last 6 bits of the second byte
    // The traditional defines for STO values assume the full byte and thus
    // the shift to pack it.
    MCELF::setOther(Data, Val >> 2);
  }

} // namespace llvm