
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

#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/Analysis/LoopInfoImpl.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/Debug.h"
using namespace llvm;

// Explicitly instantiate methods in LoopInfoImpl.h for MI-level Loops.
template class llvm::LoopBase<MachineBasicBlock, MachineLoop>;
template class llvm::LoopInfoBase<MachineBasicBlock, MachineLoop>;

char MachineLoopInfo::ID = 0;
INITIALIZE_PASS_BEGIN(MachineLoopInfo, "machine-loops",
                "Machine Natural Loop Construction", true, true)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_END(MachineLoopInfo, "machine-loops",
                "Machine Natural Loop Construction", true, true)

char &llvm::MachineLoopInfoID = MachineLoopInfo::ID;

bool MachineLoopInfo::runOnMachineFunction(MachineFunction &) {
  releaseMemory();
  LI.Analyze(getAnalysis<MachineDominatorTree>().getBase());
  return false;
}

void MachineLoopInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineDominatorTree>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

MachineBasicBlock *MachineLoop::getTopBlock() {
  MachineBasicBlock *TopMBB = getHeader();
  MachineFunction::iterator Begin = TopMBB->getParent()->begin();
  if (TopMBB != Begin) {
    MachineBasicBlock *PriorMBB = prior(MachineFunction::iterator(TopMBB));
    while (contains(PriorMBB)) {
      TopMBB = PriorMBB;
      if (TopMBB == Begin) break;
      PriorMBB = prior(MachineFunction::iterator(TopMBB));
    }
  }
  return TopMBB;
}

MachineBasicBlock *MachineLoop::getBottomBlock() {
  MachineBasicBlock *BotMBB = getHeader();
  MachineFunction::iterator End = BotMBB->getParent()->end();
  if (BotMBB != prior(End)) {
    MachineBasicBlock *NextMBB = llvm::next(MachineFunction::iterator(BotMBB));
    while (contains(NextMBB)) {
      BotMBB = NextMBB;
      if (BotMBB == llvm::next(MachineFunction::iterator(BotMBB))) break;
      NextMBB = llvm::next(MachineFunction::iterator(BotMBB));
    }
  }
  return BotMBB;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void MachineLoop::dump() const {
  print(dbgs());
}
#endif