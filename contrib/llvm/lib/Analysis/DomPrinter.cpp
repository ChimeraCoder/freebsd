
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

#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Analysis/PostDominators.h"

using namespace llvm;

namespace llvm {
template<>
struct DOTGraphTraits<DomTreeNode*> : public DefaultDOTGraphTraits {

  DOTGraphTraits (bool isSimple=false)
    : DefaultDOTGraphTraits(isSimple) {}

  std::string getNodeLabel(DomTreeNode *Node, DomTreeNode *Graph) {

    BasicBlock *BB = Node->getBlock();

    if (!BB)
      return "Post dominance root node";


    if (isSimple())
      return DOTGraphTraits<const Function*>
        ::getSimpleNodeLabel(BB, BB->getParent());
    else
      return DOTGraphTraits<const Function*>
        ::getCompleteNodeLabel(BB, BB->getParent());
  }
};

template<>
struct DOTGraphTraits<DominatorTree*> : public DOTGraphTraits<DomTreeNode*> {

  DOTGraphTraits (bool isSimple=false)
    : DOTGraphTraits<DomTreeNode*>(isSimple) {}

  static std::string getGraphName(DominatorTree *DT) {
    return "Dominator tree";
  }

  std::string getNodeLabel(DomTreeNode *Node, DominatorTree *G) {
    return DOTGraphTraits<DomTreeNode*>::getNodeLabel(Node, G->getRootNode());
  }
};

template<>
struct DOTGraphTraits<PostDominatorTree*>
  : public DOTGraphTraits<DomTreeNode*> {

  DOTGraphTraits (bool isSimple=false)
    : DOTGraphTraits<DomTreeNode*>(isSimple) {}

  static std::string getGraphName(PostDominatorTree *DT) {
    return "Post dominator tree";
  }

  std::string getNodeLabel(DomTreeNode *Node, PostDominatorTree *G ) {
    return DOTGraphTraits<DomTreeNode*>::getNodeLabel(Node, G->getRootNode());
  }
};
}

namespace {
struct DomViewer
  : public DOTGraphTraitsViewer<DominatorTree, false> {
  static char ID;
  DomViewer() : DOTGraphTraitsViewer<DominatorTree, false>("dom", ID){
    initializeDomViewerPass(*PassRegistry::getPassRegistry());
  }
};

struct DomOnlyViewer
  : public DOTGraphTraitsViewer<DominatorTree, true> {
  static char ID;
  DomOnlyViewer() : DOTGraphTraitsViewer<DominatorTree, true>("domonly", ID){
    initializeDomOnlyViewerPass(*PassRegistry::getPassRegistry());
  }
};

struct PostDomViewer
  : public DOTGraphTraitsViewer<PostDominatorTree, false> {
  static char ID;
  PostDomViewer() :
    DOTGraphTraitsViewer<PostDominatorTree, false>("postdom", ID){
      initializePostDomViewerPass(*PassRegistry::getPassRegistry());
    }
};

struct PostDomOnlyViewer
  : public DOTGraphTraitsViewer<PostDominatorTree, true> {
  static char ID;
  PostDomOnlyViewer() :
    DOTGraphTraitsViewer<PostDominatorTree, true>("postdomonly", ID){
      initializePostDomOnlyViewerPass(*PassRegistry::getPassRegistry());
    }
};
} // end anonymous namespace

char DomViewer::ID = 0;
INITIALIZE_PASS(DomViewer, "view-dom",
                "View dominance tree of function", false, false)

char DomOnlyViewer::ID = 0;
INITIALIZE_PASS(DomOnlyViewer, "view-dom-only",
                "View dominance tree of function (with no function bodies)",
                false, false)

char PostDomViewer::ID = 0;
INITIALIZE_PASS(PostDomViewer, "view-postdom",
                "View postdominance tree of function", false, false)

char PostDomOnlyViewer::ID = 0;
INITIALIZE_PASS(PostDomOnlyViewer, "view-postdom-only",
                "View postdominance tree of function "
                "(with no function bodies)",
                false, false)

namespace {
struct DomPrinter
  : public DOTGraphTraitsPrinter<DominatorTree, false> {
  static char ID;
  DomPrinter() : DOTGraphTraitsPrinter<DominatorTree, false>("dom", ID) {
    initializeDomPrinterPass(*PassRegistry::getPassRegistry());
  }
};

struct DomOnlyPrinter
  : public DOTGraphTraitsPrinter<DominatorTree, true> {
  static char ID;
  DomOnlyPrinter() : DOTGraphTraitsPrinter<DominatorTree, true>("domonly", ID) {
    initializeDomOnlyPrinterPass(*PassRegistry::getPassRegistry());
  }
};

struct PostDomPrinter
  : public DOTGraphTraitsPrinter<PostDominatorTree, false> {
  static char ID;
  PostDomPrinter() :
    DOTGraphTraitsPrinter<PostDominatorTree, false>("postdom", ID) {
      initializePostDomPrinterPass(*PassRegistry::getPassRegistry());
    }
};

struct PostDomOnlyPrinter
  : public DOTGraphTraitsPrinter<PostDominatorTree, true> {
  static char ID;
  PostDomOnlyPrinter() :
    DOTGraphTraitsPrinter<PostDominatorTree, true>("postdomonly", ID) {
      initializePostDomOnlyPrinterPass(*PassRegistry::getPassRegistry());
    }
};
} // end anonymous namespace



char DomPrinter::ID = 0;
INITIALIZE_PASS(DomPrinter, "dot-dom",
                "Print dominance tree of function to 'dot' file",
                false, false)

char DomOnlyPrinter::ID = 0;
INITIALIZE_PASS(DomOnlyPrinter, "dot-dom-only",
                "Print dominance tree of function to 'dot' file "
                "(with no function bodies)",
                false, false)

char PostDomPrinter::ID = 0;
INITIALIZE_PASS(PostDomPrinter, "dot-postdom",
                "Print postdominance tree of function to 'dot' file",
                false, false)

char PostDomOnlyPrinter::ID = 0;
INITIALIZE_PASS(PostDomOnlyPrinter, "dot-postdom-only",
                "Print postdominance tree of function to 'dot' file "
                "(with no function bodies)",
                false, false)

// Create methods available outside of this file, to use them
// "include/llvm/LinkAllPasses.h". Otherwise the pass would be deleted by
// the link time optimization.

FunctionPass *llvm::createDomPrinterPass() {
  return new DomPrinter();
}

FunctionPass *llvm::createDomOnlyPrinterPass() {
  return new DomOnlyPrinter();
}

FunctionPass *llvm::createDomViewerPass() {
  return new DomViewer();
}

FunctionPass *llvm::createDomOnlyViewerPass() {
  return new DomOnlyViewer();
}

FunctionPass *llvm::createPostDomPrinterPass() {
  return new PostDomPrinter();
}

FunctionPass *llvm::createPostDomOnlyPrinterPass() {
  return new PostDomOnlyPrinter();
}

FunctionPass *llvm::createPostDomViewerPass() {
  return new PostDomViewer();
}

FunctionPass *llvm::createPostDomOnlyViewerPass() {
  return new PostDomOnlyViewer();
}