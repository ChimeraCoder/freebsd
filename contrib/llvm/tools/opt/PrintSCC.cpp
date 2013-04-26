
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

#include "llvm/ADT/SCCIterator.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
  struct CFGSCC : public FunctionPass {
    static char ID;  // Pass identification, replacement for typeid
    CFGSCC() : FunctionPass(ID) {}
    bool runOnFunction(Function& func);

    void print(raw_ostream &O, const Module* = 0) const { }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
  };

  struct CallGraphSCC : public ModulePass {
    static char ID;  // Pass identification, replacement for typeid
    CallGraphSCC() : ModulePass(ID) {}

    // run - Print out SCCs in the call graph for the specified module.
    bool runOnModule(Module &M);

    void print(raw_ostream &O, const Module* = 0) const { }

    // getAnalysisUsage - This pass requires the CallGraph.
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<CallGraph>();
    }
  };
}

char CFGSCC::ID = 0;
static RegisterPass<CFGSCC>
Y("print-cfg-sccs", "Print SCCs of each function CFG");

char CallGraphSCC::ID = 0;
static RegisterPass<CallGraphSCC>
Z("print-callgraph-sccs", "Print SCCs of the Call Graph");

bool CFGSCC::runOnFunction(Function &F) {
  unsigned sccNum = 0;
  errs() << "SCCs for Function " << F.getName() << " in PostOrder:";
  for (scc_iterator<Function*> SCCI = scc_begin(&F),
         E = scc_end(&F); SCCI != E; ++SCCI) {
    std::vector<BasicBlock*> &nextSCC = *SCCI;
    errs() << "\nSCC #" << ++sccNum << " : ";
    for (std::vector<BasicBlock*>::const_iterator I = nextSCC.begin(),
           E = nextSCC.end(); I != E; ++I)
      errs() << (*I)->getName() << ", ";
    if (nextSCC.size() == 1 && SCCI.hasLoop())
      errs() << " (Has self-loop).";
  }
  errs() << "\n";

  return true;
}


// run - Print out SCCs in the call graph for the specified module.
bool CallGraphSCC::runOnModule(Module &M) {
  CallGraphNode* rootNode = getAnalysis<CallGraph>().getRoot();
  unsigned sccNum = 0;
  errs() << "SCCs for the program in PostOrder:";
  for (scc_iterator<CallGraphNode*> SCCI = scc_begin(rootNode),
         E = scc_end(rootNode); SCCI != E; ++SCCI) {
    const std::vector<CallGraphNode*> &nextSCC = *SCCI;
    errs() << "\nSCC #" << ++sccNum << " : ";
    for (std::vector<CallGraphNode*>::const_iterator I = nextSCC.begin(),
           E = nextSCC.end(); I != E; ++I)
      errs() << ((*I)->getFunction() ? (*I)->getFunction()->getName()
                                     : "external node") << ", ";
    if (nextSCC.size() == 1 && SCCI.hasLoop())
      errs() << " (Has self-loop).";
  }
  errs() << "\n";

  return true;
}