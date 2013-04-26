
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

#include "AMDGPU.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

/// \brief The target for the AMDGPU backend
Target llvm::TheAMDGPUTarget;

/// \brief Extern function to initialize the targets for the AMDGPU backend
extern "C" void LLVMInitializeR600TargetInfo() {
  RegisterTarget<Triple::r600, false>
    R600(TheAMDGPUTarget, "r600", "AMD GPUs HD2XXX-HD6XXX");
}