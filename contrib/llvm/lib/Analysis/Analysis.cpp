
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

#include "llvm-c/Analysis.h"
#include "llvm-c/Initialization.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/InitializePasses.h"
#include <cstring>

using namespace llvm;

/// initializeAnalysis - Initialize all passes linked into the Analysis library.
void llvm::initializeAnalysis(PassRegistry &Registry) {
  initializeAliasAnalysisAnalysisGroup(Registry);
  initializeAliasAnalysisCounterPass(Registry);
  initializeAAEvalPass(Registry);
  initializeAliasDebuggerPass(Registry);
  initializeAliasSetPrinterPass(Registry);
  initializeNoAAPass(Registry);
  initializeBasicAliasAnalysisPass(Registry);
  initializeBlockFrequencyInfoPass(Registry);
  initializeBranchProbabilityInfoPass(Registry);
  initializeCostModelAnalysisPass(Registry);
  initializeCFGViewerPass(Registry);
  initializeCFGPrinterPass(Registry);
  initializeCFGOnlyViewerPass(Registry);
  initializeCFGOnlyPrinterPass(Registry);
  initializeDependenceAnalysisPass(Registry);
  initializeDominanceFrontierPass(Registry);
  initializeDomViewerPass(Registry);
  initializeDomPrinterPass(Registry);
  initializeDomOnlyViewerPass(Registry);
  initializePostDomViewerPass(Registry);
  initializeDomOnlyPrinterPass(Registry);
  initializePostDomPrinterPass(Registry);
  initializePostDomOnlyViewerPass(Registry);
  initializePostDomOnlyPrinterPass(Registry);
  initializeIVUsersPass(Registry);
  initializeInstCountPass(Registry);
  initializeIntervalPartitionPass(Registry);
  initializeLazyValueInfoPass(Registry);
  initializeLibCallAliasAnalysisPass(Registry);
  initializeLintPass(Registry);
  initializeLoopInfoPass(Registry);
  initializeMemDepPrinterPass(Registry);
  initializeMemoryDependenceAnalysisPass(Registry);
  initializeModuleDebugInfoPrinterPass(Registry);
  initializePostDominatorTreePass(Registry);
  initializeProfileEstimatorPassPass(Registry);
  initializeNoProfileInfoPass(Registry);
  initializeNoPathProfileInfoPass(Registry);
  initializeProfileInfoAnalysisGroup(Registry);
  initializePathProfileInfoAnalysisGroup(Registry);
  initializeLoaderPassPass(Registry);
  initializePathProfileLoaderPassPass(Registry);
  initializeProfileVerifierPassPass(Registry);
  initializePathProfileVerifierPass(Registry);
  initializeProfileMetadataLoaderPassPass(Registry);
  initializeRegionInfoPass(Registry);
  initializeRegionViewerPass(Registry);
  initializeRegionPrinterPass(Registry);
  initializeRegionOnlyViewerPass(Registry);
  initializeRegionOnlyPrinterPass(Registry);
  initializeScalarEvolutionPass(Registry);
  initializeScalarEvolutionAliasAnalysisPass(Registry);
  initializeTargetTransformInfoAnalysisGroup(Registry);
  initializeTypeBasedAliasAnalysisPass(Registry);
}

void LLVMInitializeAnalysis(LLVMPassRegistryRef R) {
  initializeAnalysis(*unwrap(R));
}

LLVMBool LLVMVerifyModule(LLVMModuleRef M, LLVMVerifierFailureAction Action,
                          char **OutMessages) {
  std::string Messages;

  LLVMBool Result = verifyModule(*unwrap(M),
                            static_cast<VerifierFailureAction>(Action),
                            OutMessages? &Messages : 0);

  if (OutMessages)
    *OutMessages = strdup(Messages.c_str());

  return Result;
}

LLVMBool LLVMVerifyFunction(LLVMValueRef Fn, LLVMVerifierFailureAction Action) {
  return verifyFunction(*unwrap<Function>(Fn),
                        static_cast<VerifierFailureAction>(Action));
}

void LLVMViewFunctionCFG(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFG();
}

void LLVMViewFunctionCFGOnly(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFGOnly();
}