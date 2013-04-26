
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

#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BlockFrequencyImpl.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

INITIALIZE_PASS_BEGIN(BlockFrequencyInfo, "block-freq", "Block Frequency Analysis",
                      true, true)
INITIALIZE_PASS_DEPENDENCY(BranchProbabilityInfo)
INITIALIZE_PASS_END(BlockFrequencyInfo, "block-freq", "Block Frequency Analysis",
                    true, true)

char BlockFrequencyInfo::ID = 0;


BlockFrequencyInfo::BlockFrequencyInfo() : FunctionPass(ID) {
  initializeBlockFrequencyInfoPass(*PassRegistry::getPassRegistry());
  BFI = new BlockFrequencyImpl<BasicBlock, Function, BranchProbabilityInfo>();
}

BlockFrequencyInfo::~BlockFrequencyInfo() {
  delete BFI;
}

void BlockFrequencyInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BranchProbabilityInfo>();
  AU.setPreservesAll();
}

bool BlockFrequencyInfo::runOnFunction(Function &F) {
  BranchProbabilityInfo &BPI = getAnalysis<BranchProbabilityInfo>();
  BFI->doFunction(&F, &BPI);
  return false;
}

void BlockFrequencyInfo::print(raw_ostream &O, const Module *) const {
  if (BFI) BFI->print(O);
}

/// getblockFreq - Return block frequency. Return 0 if we don't have the
/// information. Please note that initial frequency is equal to 1024. It means
/// that we should not rely on the value itself, but only on the comparison to
/// the other block frequencies. We do this to avoid using of floating points.
///
BlockFrequency BlockFrequencyInfo::getBlockFreq(const BasicBlock *BB) const {
  return BFI->getBlockFreq(BB);
}