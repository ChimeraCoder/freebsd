
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

#define DEBUG_TYPE "strip-dead-prototypes"
#include "llvm/Transforms/IPO.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
using namespace llvm;

STATISTIC(NumDeadPrototypes, "Number of dead prototypes removed");

namespace {

/// @brief Pass to remove unused function declarations.
class StripDeadPrototypesPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  StripDeadPrototypesPass() : ModulePass(ID) {
    initializeStripDeadPrototypesPassPass(*PassRegistry::getPassRegistry());
  }
  virtual bool runOnModule(Module &M);
};

} // end anonymous namespace

char StripDeadPrototypesPass::ID = 0;
INITIALIZE_PASS(StripDeadPrototypesPass, "strip-dead-prototypes",
                "Strip Unused Function Prototypes", false, false)

bool StripDeadPrototypesPass::runOnModule(Module &M) {
  bool MadeChange = false;
  
  // Erase dead function prototypes.
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ) {
    Function *F = I++;
    // Function must be a prototype and unused.
    if (F->isDeclaration() && F->use_empty()) {
      F->eraseFromParent();
      ++NumDeadPrototypes;
      MadeChange = true;
    }
  }

  // Erase dead global var prototypes.
  for (Module::global_iterator I = M.global_begin(), E = M.global_end();
       I != E; ) {
    GlobalVariable *GV = I++;
    // Global must be a prototype and unused.
    if (GV->isDeclaration() && GV->use_empty())
      GV->eraseFromParent();
  }
  
  // Return an indication of whether we changed anything or not.
  return MadeChange;
}

ModulePass *llvm::createStripDeadPrototypesPass() {
  return new StripDeadPrototypesPass();
}