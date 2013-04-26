
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

/// initializeIPA - Initialize all passes linked into the IPA library.
void llvm::initializeIPA(PassRegistry &Registry) {
  initializeBasicCallGraphPass(Registry);
  initializeCallGraphAnalysisGroup(Registry);
  initializeCallGraphPrinterPass(Registry);
  initializeCallGraphViewerPass(Registry);
  initializeFindUsedTypesPass(Registry);
  initializeGlobalsModRefPass(Registry);
}

void LLVMInitializeIPA(LLVMPassRegistryRef R) {
  initializeIPA(*unwrap(R));
}