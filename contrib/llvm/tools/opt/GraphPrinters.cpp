
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

#include "llvm/Analysis/Dominators.h"
#include "llvm/Pass.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
//                            DomInfoPrinter Pass
//===----------------------------------------------------------------------===//

namespace {
  class DomInfoPrinter : public FunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid
    DomInfoPrinter() : FunctionPass(ID) {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<DominatorTree>();

    }

    virtual bool runOnFunction(Function &F) {
      getAnalysis<DominatorTree>().dump();
      return false;
    }
  };
}

char DomInfoPrinter::ID = 0;
static RegisterPass<DomInfoPrinter>
DIP("print-dom-info", "Dominator Info Printer", true, true);