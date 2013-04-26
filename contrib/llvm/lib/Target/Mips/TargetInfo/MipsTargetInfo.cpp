
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

#include "Mips.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheMipsTarget, llvm::TheMipselTarget;
Target llvm::TheMips64Target, llvm::TheMips64elTarget;

extern "C" void LLVMInitializeMipsTargetInfo() {
  RegisterTarget<Triple::mips,
        /*HasJIT=*/true> X(TheMipsTarget, "mips", "Mips");

  RegisterTarget<Triple::mipsel,
        /*HasJIT=*/true> Y(TheMipselTarget, "mipsel", "Mipsel");

  RegisterTarget<Triple::mips64,
        /*HasJIT=*/false> A(TheMips64Target, "mips64", "Mips64 [experimental]");

  RegisterTarget<Triple::mips64el,
        /*HasJIT=*/false> B(TheMips64elTarget,
                            "mips64el", "Mips64el [experimental]");
}