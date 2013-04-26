
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

#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

namespace {
/// \brief A nonce module pass used to place a barrier in a pass manager.
///
/// There is no mechanism for ending a CGSCC pass manager once one is started.
/// This prevents extension points from having clear deterministic ordering
/// when they are phrased as non-module passes.
class BarrierNoop : public ModulePass {
public:
  static char ID; // Pass identification.

  BarrierNoop() : ModulePass(ID) {
    initializeBarrierNoopPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) { return false; }
};
}

ModulePass *llvm::createBarrierNoopPass() { return new BarrierNoop(); }

char BarrierNoop::ID = 0;
INITIALIZE_PASS(BarrierNoop, "barrier", "A No-Op Barrier Pass",
                false, false)