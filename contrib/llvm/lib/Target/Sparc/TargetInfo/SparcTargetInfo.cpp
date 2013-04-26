
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

#include "Sparc.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheSparcTarget;
Target llvm::TheSparcV9Target;

extern "C" void LLVMInitializeSparcTargetInfo() { 
  RegisterTarget<Triple::sparc> X(TheSparcTarget, "sparc", "Sparc");
  RegisterTarget<Triple::sparcv9> Y(TheSparcV9Target, "sparcv9", "Sparc V9");
}