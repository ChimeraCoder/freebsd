
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

#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/MC/MCSymbol.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
// MachineModuleInfoMachO
//===----------------------------------------------------------------------===//

// Out of line virtual method.
void MachineModuleInfoMachO::anchor() {}
void MachineModuleInfoELF::anchor() {}

static int SortSymbolPair(const void *LHS, const void *RHS) {
  typedef std::pair<MCSymbol*, MachineModuleInfoImpl::StubValueTy> PairTy;
  const MCSymbol *LHSS = ((const PairTy *)LHS)->first;
  const MCSymbol *RHSS = ((const PairTy *)RHS)->first;
  return LHSS->getName().compare(RHSS->getName());
}

/// GetSortedStubs - Return the entries from a DenseMap in a deterministic
/// sorted orer.
MachineModuleInfoImpl::SymbolListTy
MachineModuleInfoImpl::GetSortedStubs(const DenseMap<MCSymbol*,
                                      MachineModuleInfoImpl::StubValueTy>&Map) {
  MachineModuleInfoImpl::SymbolListTy List(Map.begin(), Map.end());

  if (!List.empty())
    qsort(&List[0], List.size(), sizeof(List[0]), SortSymbolPair);
  return List;
}