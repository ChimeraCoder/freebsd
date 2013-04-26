
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

#include "llvm/InitializePasses.h"
#include "llvm-c/Initialization.h"

using namespace llvm;

/// initializeTransformUtils - Initialize all passes in the TransformUtils
/// library.
void llvm::initializeTransformUtils(PassRegistry &Registry) {
  initializeBreakCriticalEdgesPass(Registry);
  initializeInstNamerPass(Registry);
  initializeLCSSAPass(Registry);
  initializeLoopSimplifyPass(Registry);
  initializeLowerInvokePass(Registry);
  initializeLowerSwitchPass(Registry);
  initializePromotePassPass(Registry);
  initializeUnifyFunctionExitNodesPass(Registry);
  initializeInstSimplifierPass(Registry);
  initializeMetaRenamerPass(Registry);
}

/// LLVMInitializeTransformUtils - C binding for initializeTransformUtilsPasses.
void LLVMInitializeTransformUtils(LLVMPassRegistryRef R) {
  initializeTransformUtils(*unwrap(R));
}