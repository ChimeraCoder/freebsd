
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

#include "NVPTXSplitBBatBar.h"
#include "NVPTXUtilities.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/InstIterator.h"

using namespace llvm;

namespace llvm { FunctionPass *createSplitBBatBarPass(); }

char NVPTXSplitBBatBar::ID = 0;

bool NVPTXSplitBBatBar::runOnFunction(Function &F) {

  SmallVector<Instruction *, 4> SplitPoints;
  bool changed = false;

  // Collect all the split points in SplitPoints
  for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
    BasicBlock::iterator IB = BI->begin();
    BasicBlock::iterator II = IB;
    BasicBlock::iterator IE = BI->end();

    // Skit the first intruction. No splitting is needed at this
    // point even if this is a bar.
    while (II != IE) {
      if (IntrinsicInst *inst = dyn_cast<IntrinsicInst>(II)) {
        Intrinsic::ID id = inst->getIntrinsicID();
        // If this is a barrier, split at this instruction
        // and the next instruction.
        if (llvm::isBarrierIntrinsic(id)) {
          if (II != IB)
            SplitPoints.push_back(II);
          II++;
          if ((II != IE) && (!II->isTerminator())) {
            SplitPoints.push_back(II);
            II++;
          }
          continue;
        }
      }
      II++;
    }
  }

  for (unsigned i = 0; i != SplitPoints.size(); i++) {
    changed = true;
    Instruction *inst = SplitPoints[i];
    inst->getParent()->splitBasicBlock(inst, "bar_split");
  }

  return changed;
}

// This interface will most likely not be necessary, because this pass will
// not be invoked by the driver, but will be used as a prerequisite to
// another pass.
FunctionPass *llvm::createSplitBBatBarPass() { return new NVPTXSplitBBatBar(); }