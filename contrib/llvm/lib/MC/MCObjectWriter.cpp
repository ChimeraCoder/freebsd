
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

#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbol.h"

using namespace llvm;

MCObjectWriter::~MCObjectWriter() {
}

bool
MCObjectWriter::IsSymbolRefDifferenceFullyResolved(const MCAssembler &Asm,
                                                   const MCSymbolRefExpr *A,
                                                   const MCSymbolRefExpr *B,
                                                   bool InSet) const {
  // Modified symbol references cannot be resolved.
  if (A->getKind() != MCSymbolRefExpr::VK_None ||
      B->getKind() != MCSymbolRefExpr::VK_None)
    return false;

  const MCSymbol &SA = A->getSymbol();
  const MCSymbol &SB = B->getSymbol();
  if (SA.AliasedSymbol().isUndefined() || SB.AliasedSymbol().isUndefined())
    return false;

  const MCSymbolData &DataA = Asm.getSymbolData(SA);
  const MCSymbolData &DataB = Asm.getSymbolData(SB);
  if(!DataA.getFragment() || !DataB.getFragment())
    return false;

  return IsSymbolRefDifferenceFullyResolvedImpl(Asm, DataA,
                                                *DataB.getFragment(),
                                                InSet,
                                                false);
}

bool
MCObjectWriter::IsSymbolRefDifferenceFullyResolvedImpl(const MCAssembler &Asm,
                                                      const MCSymbolData &DataA,
                                                      const MCFragment &FB,
                                                      bool InSet,
                                                      bool IsPCRel) const {
  const MCSection &SecA = DataA.getSymbol().AliasedSymbol().getSection();
  const MCSection &SecB = FB.getParent()->getSection();
  // On ELF and COFF  A - B is absolute if A and B are in the same section.
  return &SecA == &SecB;
}