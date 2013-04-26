
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

#include "ObjCARC.h"
#include "llvm-c/Core.h"
#include "llvm-c/Initialization.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

namespace llvm {
  class PassRegistry;
}

using namespace llvm;
using namespace llvm::objcarc;

/// \brief A handy option to enable/disable all ARC Optimizations.
bool llvm::objcarc::EnableARCOpts;
static cl::opt<bool, true>
EnableARCOptimizations("enable-objc-arc-opts",
                       cl::location(EnableARCOpts),
                       cl::init(true));

/// initializeObjCARCOptsPasses - Initialize all passes linked into the
/// ObjCARCOpts library.
void llvm::initializeObjCARCOpts(PassRegistry &Registry) {
  initializeObjCARCAliasAnalysisPass(Registry);
  initializeObjCARCAPElimPass(Registry);
  initializeObjCARCExpandPass(Registry);
  initializeObjCARCContractPass(Registry);
  initializeObjCARCOptPass(Registry);
}

void LLVMInitializeObjCARCOpts(LLVMPassRegistryRef R) {
  initializeObjCARCOpts(*unwrap(R));
}