
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

#define DEBUG_TYPE "block-placement"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CFG.h"
#include <set>
using namespace llvm;

STATISTIC(NumMoved, "Number of basic blocks moved");

namespace {
  struct BlockPlacement : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    BlockPlacement() : FunctionPass(ID) {
      initializeBlockPlacementPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
      AU.addRequired<ProfileInfo>();
      //AU.addPreserved<ProfileInfo>();  // Does this work?
    }
  private:
    /// PI - The profile information that is guiding us.
    ///
    ProfileInfo *PI;

    /// NumMovedBlocks - Every time we move a block, increment this counter.
    ///
    unsigned NumMovedBlocks;

    /// PlacedBlocks - Every time we place a block, remember it so we don't get
    /// into infinite loops.
    std::set<BasicBlock*> PlacedBlocks;

    /// InsertPos - This an iterator to the next place we want to insert a
    /// block.
    Function::iterator InsertPos;

    /// PlaceBlocks - Recursively place the specified blocks and any unplaced
    /// successors.
    void PlaceBlocks(BasicBlock *BB);
  };
}

char BlockPlacement::ID = 0;
INITIALIZE_PASS_BEGIN(BlockPlacement, "block-placement",
                "Profile Guided Basic Block Placement", false, false)
INITIALIZE_AG_DEPENDENCY(ProfileInfo)
INITIALIZE_PASS_END(BlockPlacement, "block-placement",
                "Profile Guided Basic Block Placement", false, false)

FunctionPass *llvm::createBlockPlacementPass() { return new BlockPlacement(); }

bool BlockPlacement::runOnFunction(Function &F) {
  PI = &getAnalysis<ProfileInfo>();

  NumMovedBlocks = 0;
  InsertPos = F.begin();

  // Recursively place all blocks.
  PlaceBlocks(F.begin());

  PlacedBlocks.clear();
  NumMoved += NumMovedBlocks;
  return NumMovedBlocks != 0;
}


/// PlaceBlocks - Recursively place the specified blocks and any unplaced
/// successors.
void BlockPlacement::PlaceBlocks(BasicBlock *BB) {
  assert(!PlacedBlocks.count(BB) && "Already placed this block!");
  PlacedBlocks.insert(BB);

  // Place the specified block.
  if (&*InsertPos != BB) {
    // Use splice to move the block into the right place.  This avoids having to
    // remove the block from the function then readd it, which causes a bunch of
    // symbol table traffic that is entirely pointless.
    Function::BasicBlockListType &Blocks = BB->getParent()->getBasicBlockList();
    Blocks.splice(InsertPos, Blocks, BB);

    ++NumMovedBlocks;
  } else {
    // This block is already in the right place, we don't have to do anything.
    ++InsertPos;
  }

  // Keep placing successors until we run out of ones to place.  Note that this
  // loop is very inefficient (N^2) for blocks with many successors, like switch
  // statements.  FIXME!
  while (1) {
    // Okay, now place any unplaced successors.
    succ_iterator SI = succ_begin(BB), E = succ_end(BB);

    // Scan for the first unplaced successor.
    for (; SI != E && PlacedBlocks.count(*SI); ++SI)
      /*empty*/;
    if (SI == E) return;  // No more successors to place.

    double MaxExecutionCount = PI->getExecutionCount(*SI);
    BasicBlock *MaxSuccessor = *SI;

    // Scan for more frequently executed successors
    for (; SI != E; ++SI)
      if (!PlacedBlocks.count(*SI)) {
        double Count = PI->getExecutionCount(*SI);
        if (Count > MaxExecutionCount ||
            // Prefer to not disturb the code.
            (Count == MaxExecutionCount && *SI == &*InsertPos)) {
          MaxExecutionCount = Count;
          MaxSuccessor = *SI;
        }
      }

    // Now that we picked the maximally executed successor, place it.
    PlaceBlocks(MaxSuccessor);
  }
}