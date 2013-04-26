
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

#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCValue.h"

using namespace llvm;

MCELFObjectTargetWriter::MCELFObjectTargetWriter(bool Is64Bit_,
                                                 uint8_t OSABI_,
                                                 uint16_t EMachine_,
                                                 bool HasRelocationAddend_,
                                                 bool IsN64_)
  : OSABI(OSABI_), EMachine(EMachine_),
    HasRelocationAddend(HasRelocationAddend_), Is64Bit(Is64Bit_),
    IsN64(IsN64_){
}

const MCSymbol *MCELFObjectTargetWriter::ExplicitRelSym(const MCAssembler &Asm,
                                                        const MCValue &Target,
                                                        const MCFragment &F,
                                                        const MCFixup &Fixup,
                                                        bool IsPCRel) const {
  return NULL;
}

const MCSymbol *MCELFObjectTargetWriter::undefinedExplicitRelSym(const MCValue &Target,
                                                                 const MCFixup &Fixup,
                                                                 bool IsPCRel) const {
  const MCSymbol &Symbol = Target.getSymA()->getSymbol();
  return &Symbol.AliasedSymbol();
}

void MCELFObjectTargetWriter::adjustFixupOffset(const MCFixup &Fixup,
                                                uint64_t &RelocOffset) {
}

void
MCELFObjectTargetWriter::sortRelocs(const MCAssembler &Asm,
                                    std::vector<ELFRelocationEntry> &Relocs) {
  // Sort by the r_offset, just like gnu as does.
  array_pod_sort(Relocs.begin(), Relocs.end());
}