
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

#include "llvm/CodeGen/MachinePostDominators.h"

using namespace llvm;

char MachinePostDominatorTree::ID = 0;

//declare initializeMachinePostDominatorTreePass
INITIALIZE_PASS(MachinePostDominatorTree, "machinepostdomtree",
                "MachinePostDominator Tree Construction", true, true)

MachinePostDominatorTree::MachinePostDominatorTree() : MachineFunctionPass(ID) {
  initializeMachinePostDominatorTreePass(*PassRegistry::getPassRegistry());
  DT = new DominatorTreeBase<MachineBasicBlock>(true); //true indicate
                                                       // postdominator
}

FunctionPass *
MachinePostDominatorTree::createMachinePostDominatorTreePass() {
  return new MachinePostDominatorTree();
}

bool
MachinePostDominatorTree::runOnMachineFunction(MachineFunction &F) {
  DT->recalculate(F);
  return false;
}

MachinePostDominatorTree::~MachinePostDominatorTree() {
  delete DT;
}

void
MachinePostDominatorTree::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
}

void
MachinePostDominatorTree::print(llvm::raw_ostream &OS, const Module *M) const {
  DT->print(OS);
}