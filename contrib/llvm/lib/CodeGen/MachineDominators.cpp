
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

#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/Passes.h"

using namespace llvm;

namespace llvm {
TEMPLATE_INSTANTIATION(class DomTreeNodeBase<MachineBasicBlock>);
TEMPLATE_INSTANTIATION(class DominatorTreeBase<MachineBasicBlock>);
}

char MachineDominatorTree::ID = 0;

INITIALIZE_PASS(MachineDominatorTree, "machinedomtree",
                "MachineDominator Tree Construction", true, true)

char &llvm::MachineDominatorsID = MachineDominatorTree::ID;

void MachineDominatorTree::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool MachineDominatorTree::runOnMachineFunction(MachineFunction &F) {
  DT->recalculate(F);

  return false;
}

MachineDominatorTree::MachineDominatorTree()
    : MachineFunctionPass(ID) {
  initializeMachineDominatorTreePass(*PassRegistry::getPassRegistry());
  DT = new DominatorTreeBase<MachineBasicBlock>(false);
}

MachineDominatorTree::~MachineDominatorTree() {
  delete DT;
}

void MachineDominatorTree::releaseMemory() {
  DT->releaseMemory();
}

void MachineDominatorTree::print(raw_ostream &OS, const Module*) const {
  DT->print(OS);
}