
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

#include "X86.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheX86_32Target, llvm::TheX86_64Target;

extern "C" void LLVMInitializeX86TargetInfo() { 
  RegisterTarget<Triple::x86, /*HasJIT=*/true>
    X(TheX86_32Target, "x86", "32-bit X86: Pentium-Pro and above");

  RegisterTarget<Triple::x86_64, /*HasJIT=*/true>
    Y(TheX86_64Target, "x86-64", "64-bit X86: EM64T and AMD64");
}