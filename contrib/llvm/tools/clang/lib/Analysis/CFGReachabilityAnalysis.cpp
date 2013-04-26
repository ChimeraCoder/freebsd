
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

#include "llvm/ADT/SmallVector.h"
#include "clang/Analysis/Analyses/CFGReachabilityAnalysis.h"
#include "clang/Analysis/CFG.h"

using namespace clang;

CFGReverseBlockReachabilityAnalysis::CFGReverseBlockReachabilityAnalysis(const CFG &cfg)
  : analyzed(cfg.getNumBlockIDs(), false) {}

bool CFGReverseBlockReachabilityAnalysis::isReachable(const CFGBlock *Src,
                                          const CFGBlock *Dst) {

  const unsigned DstBlockID = Dst->getBlockID();
  
  // If we haven't analyzed the destination node, run the analysis now
  if (!analyzed[DstBlockID]) {
    mapReachability(Dst);
    analyzed[DstBlockID] = true;
  }
  
  // Return the cached result
  return reachable[DstBlockID][Src->getBlockID()];
}

// Maps reachability to a common node by walking the predecessors of the
// destination node.
void CFGReverseBlockReachabilityAnalysis::mapReachability(const CFGBlock *Dst) {
  SmallVector<const CFGBlock *, 11> worklist;
  llvm::BitVector visited(analyzed.size());
  
  ReachableSet &DstReachability = reachable[Dst->getBlockID()];
  DstReachability.resize(analyzed.size(), false);
  
  // Start searching from the destination node, since we commonly will perform
  // multiple queries relating to a destination node.
  worklist.push_back(Dst);
  bool firstRun = true;
  
  while (!worklist.empty()) {    
    const CFGBlock *block = worklist.back();
    worklist.pop_back();
    
    if (visited[block->getBlockID()])
      continue;
    visited[block->getBlockID()] = true;
    
    // Update reachability information for this node -> Dst
    if (!firstRun) {
      // Don't insert Dst -> Dst unless it was a predecessor of itself
      DstReachability[block->getBlockID()] = true;
    }
    else
      firstRun = false;
    
    // Add the predecessors to the worklist.
    for (CFGBlock::const_pred_iterator i = block->pred_begin(), 
         e = block->pred_end(); i != e; ++i) {
      worklist.push_back(*i);
    }
  }
}