
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

#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
  /// ExternalFunctionsPassedConstants - This pass prints out call sites to
  /// external functions that are called with constant arguments.  This can be
  /// useful when looking for standard library functions we should constant fold
  /// or handle in alias analyses.
  struct ExternalFunctionsPassedConstants : public ModulePass {
    static char ID; // Pass ID, replacement for typeid
    ExternalFunctionsPassedConstants() : ModulePass(ID) {}
    virtual bool runOnModule(Module &M) {
      for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
        if (!I->isDeclaration()) continue;
        
        bool PrintedFn = false;
        for (Value::use_iterator UI = I->use_begin(), E = I->use_end();
             UI != E; ++UI) {
          Instruction *User = dyn_cast<Instruction>(*UI);
          if (!User) continue;
          
          CallSite CS(cast<Value>(User));
          if (!CS) continue;
          
          for (CallSite::arg_iterator AI = CS.arg_begin(),
               E = CS.arg_end(); AI != E; ++AI) {
            if (!isa<Constant>(*AI)) continue;

            if (!PrintedFn) {
              errs() << "Function '" << I->getName() << "':\n";
              PrintedFn = true;
            }
            errs() << *User;
            break;
          }
        }
      }

      return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
  };
}

char ExternalFunctionsPassedConstants::ID = 0;
static RegisterPass<ExternalFunctionsPassedConstants>
  P1("print-externalfnconstants",
     "Print external fn callsites passed constants");

namespace {
  struct CallGraphPrinter : public ModulePass {
    static char ID; // Pass ID, replacement for typeid
    CallGraphPrinter() : ModulePass(ID) {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequiredTransitive<CallGraph>();
    }
    virtual bool runOnModule(Module &M) {
      getAnalysis<CallGraph>().print(errs(), &M);
      return false;
    }
  };
}

char CallGraphPrinter::ID = 0;
static RegisterPass<CallGraphPrinter>
  P2("print-callgraph", "Print a call graph");