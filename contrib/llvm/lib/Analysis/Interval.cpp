
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

#include "llvm/Analysis/Interval.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

using namespace llvm;

//===----------------------------------------------------------------------===//
// Interval Implementation
//===----------------------------------------------------------------------===//

// isLoop - Find out if there is a back edge in this interval...
//
bool Interval::isLoop() const {
  // There is a loop in this interval iff one of the predecessors of the header
  // node lives in the interval.
  for (::pred_iterator I = ::pred_begin(HeaderNode), E = ::pred_end(HeaderNode);
       I != E; ++I)
    if (contains(*I))
      return true;
  return false;
}


void Interval::print(raw_ostream &OS) const {
  OS << "-------------------------------------------------------------\n"
       << "Interval Contents:\n";

  // Print out all of the basic blocks in the interval...
  for (std::vector<BasicBlock*>::const_iterator I = Nodes.begin(),
         E = Nodes.end(); I != E; ++I)
    OS << **I << "\n";

  OS << "Interval Predecessors:\n";
  for (std::vector<BasicBlock*>::const_iterator I = Predecessors.begin(),
         E = Predecessors.end(); I != E; ++I)
    OS << **I << "\n";

  OS << "Interval Successors:\n";
  for (std::vector<BasicBlock*>::const_iterator I = Successors.begin(),
         E = Successors.end(); I != E; ++I)
    OS << **I << "\n";
}