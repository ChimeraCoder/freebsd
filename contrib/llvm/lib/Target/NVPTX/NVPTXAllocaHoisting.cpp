
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

#include "NVPTXAllocaHoisting.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

namespace llvm {

bool NVPTXAllocaHoisting::runOnFunction(Function &function) {
  bool functionModified = false;
  Function::iterator I = function.begin();
  TerminatorInst *firstTerminatorInst = (I++)->getTerminator();

  for (Function::iterator E = function.end(); I != E; ++I) {
    for (BasicBlock::iterator BI = I->begin(), BE = I->end(); BI != BE;) {
      AllocaInst *allocaInst = dyn_cast<AllocaInst>(BI++);
      if (allocaInst && isa<ConstantInt>(allocaInst->getArraySize())) {
        allocaInst->moveBefore(firstTerminatorInst);
        functionModified = true;
      }
    }
  }

  return functionModified;
}

char NVPTXAllocaHoisting::ID = 1;
RegisterPass<NVPTXAllocaHoisting>
X("alloca-hoisting", "Hoisting alloca instructions in non-entry "
                     "blocks to the entry block");

FunctionPass *createAllocaHoisting() { return new NVPTXAllocaHoisting(); }

} // end namespace llvm