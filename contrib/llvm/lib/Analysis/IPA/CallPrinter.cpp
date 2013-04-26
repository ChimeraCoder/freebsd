
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
#include "llvm/Analysis/CallPrinter.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"

using namespace llvm;

namespace llvm {

template<>
struct DOTGraphTraits<CallGraph*> : public DefaultDOTGraphTraits {
  DOTGraphTraits (bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}

  static std::string getGraphName(CallGraph *Graph) {
    return "Call graph";
  }

  std::string getNodeLabel(CallGraphNode *Node, CallGraph *Graph) {
    if (Function *Func = Node->getFunction())
      return Func->getName();

    return "external node";
  }
};

} // end llvm namespace

namespace {

struct CallGraphViewer
  : public DOTGraphTraitsModuleViewer<CallGraph, true> {
  static char ID;

  CallGraphViewer()
    : DOTGraphTraitsModuleViewer<CallGraph, true>("callgraph", ID) {
    initializeCallGraphViewerPass(*PassRegistry::getPassRegistry());
  }
};

struct CallGraphPrinter
  : public DOTGraphTraitsModulePrinter<CallGraph, true> {
  static char ID;

  CallGraphPrinter()
    : DOTGraphTraitsModulePrinter<CallGraph, true>("callgraph", ID) {
      initializeCallGraphPrinterPass(*PassRegistry::getPassRegistry());
  }
};

} // end anonymous namespace

char CallGraphViewer::ID = 0;
INITIALIZE_PASS(CallGraphViewer, "view-callgraph",
                "View call graph",
                false, false)

char CallGraphPrinter::ID = 0;
INITIALIZE_PASS(CallGraphPrinter, "dot-callgraph",
                "Print call graph to 'dot' file",
                false, false)

// Create methods available outside of this file, to use them
// "include/llvm/LinkAllPasses.h". Otherwise the pass would be deleted by
// the link time optimization.

ModulePass *llvm::createCallGraphViewerPass() {
  return new CallGraphViewer();
}

ModulePass *llvm::createCallGraphPrinterPass() {
  return new CallGraphPrinter();
}