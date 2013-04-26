
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

#define DEBUG_TYPE "postdomtree"

#include "llvm/Analysis/PostDominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/Analysis/DominatorInternals.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
//  PostDominatorTree Implementation
//===----------------------------------------------------------------------===//

char PostDominatorTree::ID = 0;
INITIALIZE_PASS(PostDominatorTree, "postdomtree",
                "Post-Dominator Tree Construction", true, true)

bool PostDominatorTree::runOnFunction(Function &F) {
  DT->recalculate(F);
  return false;
}

PostDominatorTree::~PostDominatorTree() {
  delete DT;
}

void PostDominatorTree::print(raw_ostream &OS, const Module *) const {
  DT->print(OS);
}


FunctionPass* llvm::createPostDomTree() {
  return new PostDominatorTree();
}