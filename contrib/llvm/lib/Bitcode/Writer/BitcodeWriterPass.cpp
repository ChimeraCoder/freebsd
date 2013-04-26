
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

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Pass.h"
using namespace llvm;

namespace {
  class WriteBitcodePass : public ModulePass {
    raw_ostream &OS; // raw_ostream to print on
  public:
    static char ID; // Pass identification, replacement for typeid
    explicit WriteBitcodePass(raw_ostream &o)
      : ModulePass(ID), OS(o) {}

    const char *getPassName() const { return "Bitcode Writer"; }

    bool runOnModule(Module &M) {
      WriteBitcodeToFile(&M, OS);
      return false;
    }
  };
}

char WriteBitcodePass::ID = 0;

/// createBitcodeWriterPass - Create and return a pass that writes the module
/// to the specified ostream.
ModulePass *llvm::createBitcodeWriterPass(raw_ostream &Str) {
  return new WriteBitcodePass(Str);
}