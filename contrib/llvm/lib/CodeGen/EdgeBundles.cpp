
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

#include "llvm/CodeGen/EdgeBundles.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/GraphWriter.h"

using namespace llvm;

static cl::opt<bool>
ViewEdgeBundles("view-edge-bundles", cl::Hidden,
                cl::desc("Pop up a window to show edge bundle graphs"));

char EdgeBundles::ID = 0;

INITIALIZE_PASS(EdgeBundles, "edge-bundles", "Bundle Machine CFG Edges",
                /* cfg = */true, /* analysis = */ true)

char &llvm::EdgeBundlesID = EdgeBundles::ID;

void EdgeBundles::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool EdgeBundles::runOnMachineFunction(MachineFunction &mf) {
  MF = &mf;
  EC.clear();
  EC.grow(2 * MF->getNumBlockIDs());

  for (MachineFunction::const_iterator I = MF->begin(), E = MF->end(); I != E;
       ++I) {
    const MachineBasicBlock &MBB = *I;
    unsigned OutE = 2 * MBB.getNumber() + 1;
    // Join the outgoing bundle with the ingoing bundles of all successors.
    for (MachineBasicBlock::const_succ_iterator SI = MBB.succ_begin(),
           SE = MBB.succ_end(); SI != SE; ++SI)
      EC.join(OutE, 2 * (*SI)->getNumber());
  }
  EC.compress();
  if (ViewEdgeBundles)
    view();

  // Compute the reverse mapping.
  Blocks.clear();
  Blocks.resize(getNumBundles());

  for (unsigned i = 0, e = MF->getNumBlockIDs(); i != e; ++i) {
    unsigned b0 = getBundle(i, 0);
    unsigned b1 = getBundle(i, 1);
    Blocks[b0].push_back(i);
    if (b1 != b0)
      Blocks[b1].push_back(i);
  }

  return false;
}

/// view - Visualize the annotated bipartite CFG with Graphviz.
void EdgeBundles::view() const {
  ViewGraph(*this, "EdgeBundles");
}

/// Specialize WriteGraph, the standard implementation won't work.
raw_ostream &llvm::WriteGraph(raw_ostream &O, const EdgeBundles &G,
                              bool ShortNames,
                              const Twine &Title) {
  const MachineFunction *MF = G.getMachineFunction();

  O << "digraph {\n";
  for (MachineFunction::const_iterator I = MF->begin(), E = MF->end();
       I != E; ++I) {
    unsigned BB = I->getNumber();
    O << "\t\"BB#" << BB << "\" [ shape=box ]\n"
      << '\t' << G.getBundle(BB, false) << " -> \"BB#" << BB << "\"\n"
      << "\t\"BB#" << BB << "\" -> " << G.getBundle(BB, true) << '\n';
    for (MachineBasicBlock::const_succ_iterator SI = I->succ_begin(),
           SE = I->succ_end(); SI != SE; ++SI)
      O << "\t\"BB#" << BB << "\" -> \"BB#" << (*SI)->getNumber()
        << "\" [ color=lightgray ]\n";
  }
  O << "}\n";
  return O;
}