
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

#include "llvm-c/Initialization.h"
#include "llvm-c/Transforms/IPO.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

void llvm::initializeIPO(PassRegistry &Registry) {
  initializeArgPromotionPass(Registry);
  initializeConstantMergePass(Registry);
  initializeDAEPass(Registry);
  initializeDAHPass(Registry);
  initializeFunctionAttrsPass(Registry);
  initializeGlobalDCEPass(Registry);
  initializeGlobalOptPass(Registry);
  initializeIPCPPass(Registry);
  initializeAlwaysInlinerPass(Registry);
  initializeSimpleInlinerPass(Registry);
  initializeInternalizePassPass(Registry);
  initializeLoopExtractorPass(Registry);
  initializeBlockExtractorPassPass(Registry);
  initializeSingleLoopExtractorPass(Registry);
  initializeMergeFunctionsPass(Registry);
  initializePartialInlinerPass(Registry);
  initializePruneEHPass(Registry);
  initializeStripDeadPrototypesPassPass(Registry);
  initializeStripSymbolsPass(Registry);
  initializeStripDebugDeclarePass(Registry);
  initializeStripDeadDebugInfoPass(Registry);
  initializeStripNonDebugSymbolsPass(Registry);
}

void LLVMInitializeIPO(LLVMPassRegistryRef R) {
  initializeIPO(*unwrap(R));
}

void LLVMAddArgumentPromotionPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createArgumentPromotionPass());
}

void LLVMAddConstantMergePass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createConstantMergePass());
}

void LLVMAddDeadArgEliminationPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createDeadArgEliminationPass());
}

void LLVMAddFunctionAttrsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createFunctionAttrsPass());
}

void LLVMAddFunctionInliningPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createFunctionInliningPass());
}

void LLVMAddAlwaysInlinerPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(llvm::createAlwaysInlinerPass());
}

void LLVMAddGlobalDCEPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createGlobalDCEPass());
}

void LLVMAddGlobalOptimizerPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createGlobalOptimizerPass());
}

void LLVMAddIPConstantPropagationPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createIPConstantPropagationPass());
}

void LLVMAddPruneEHPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createPruneEHPass());
}

void LLVMAddIPSCCPPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createIPSCCPPass());
}

void LLVMAddInternalizePass(LLVMPassManagerRef PM, unsigned AllButMain) {
  std::vector<const char *> Export;
  if (AllButMain)
    Export.push_back("main");
  unwrap(PM)->add(createInternalizePass(Export));
}

void LLVMAddStripDeadPrototypesPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createStripDeadPrototypesPass());
}

void LLVMAddStripSymbolsPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createStripSymbolsPass());
}