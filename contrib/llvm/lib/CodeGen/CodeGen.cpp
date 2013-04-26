
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

/// initializeCodeGen - Initialize all passes linked into the CodeGen library.
void llvm::initializeCodeGen(PassRegistry &Registry) {
  initializeBasicTTIPass(Registry);
  initializeBranchFolderPassPass(Registry);
  initializeCalculateSpillWeightsPass(Registry);
  initializeDeadMachineInstructionElimPass(Registry);
  initializeEarlyIfConverterPass(Registry);
  initializeExpandPostRAPass(Registry);
  initializeExpandISelPseudosPass(Registry);
  initializeFinalizeMachineBundlesPass(Registry);
  initializeGCMachineCodeAnalysisPass(Registry);
  initializeGCModuleInfoPass(Registry);
  initializeIfConverterPass(Registry);
  initializeLiveDebugVariablesPass(Registry);
  initializeLiveIntervalsPass(Registry);
  initializeLiveStacksPass(Registry);
  initializeLiveVariablesPass(Registry);
  initializeLocalStackSlotPassPass(Registry);
  initializeMachineBlockFrequencyInfoPass(Registry);
  initializeMachineBlockPlacementPass(Registry);
  initializeMachineBlockPlacementStatsPass(Registry);
  initializeMachineCopyPropagationPass(Registry);
  initializeMachineCSEPass(Registry);
  initializeMachineDominatorTreePass(Registry);
  initializeMachinePostDominatorTreePass(Registry);
  initializeMachineLICMPass(Registry);
  initializeMachineLoopInfoPass(Registry);
  initializeMachineModuleInfoPass(Registry);
  initializeMachineSchedulerPass(Registry);
  initializeMachineSinkingPass(Registry);
  initializeMachineVerifierPassPass(Registry);
  initializeOptimizePHIsPass(Registry);
  initializePHIEliminationPass(Registry);
  initializePeepholeOptimizerPass(Registry);
  initializePostRASchedulerPass(Registry);
  initializeProcessImplicitDefsPass(Registry);
  initializePEIPass(Registry);
  initializeRegisterCoalescerPass(Registry);
  initializeSlotIndexesPass(Registry);
  initializeStackProtectorPass(Registry);
  initializeStackColoringPass(Registry);
  initializeStackSlotColoringPass(Registry);
  initializeStrongPHIEliminationPass(Registry);
  initializeTailDuplicatePassPass(Registry);
  initializeTargetPassConfigPass(Registry);
  initializeTwoAddressInstructionPassPass(Registry);
  initializeUnpackMachineBundlesPass(Registry);
  initializeUnreachableBlockElimPass(Registry);
  initializeUnreachableMachineBlockElimPass(Registry);
  initializeVirtRegMapPass(Registry);
  initializeVirtRegRewriterPass(Registry);
  initializeLowerIntrinsicsPass(Registry);
  initializeMachineFunctionPrinterPassPass(Registry);
}

void LLVMInitializeCodeGen(LLVMPassRegistryRef R) {
  initializeCodeGen(*unwrap(R));
}