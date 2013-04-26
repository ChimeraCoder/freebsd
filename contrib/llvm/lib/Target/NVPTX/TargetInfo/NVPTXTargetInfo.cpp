
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

#include "NVPTX.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheNVPTXTarget32;
Target llvm::TheNVPTXTarget64;

extern "C" void LLVMInitializeNVPTXTargetInfo() {
  RegisterTarget<Triple::nvptx> X(TheNVPTXTarget32, "nvptx",
                                  "NVIDIA PTX 32-bit");
  RegisterTarget<Triple::nvptx64> Y(TheNVPTXTarget64, "nvptx64",
                                    "NVIDIA PTX 64-bit");
}