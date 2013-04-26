
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

#define DEBUG_TYPE "livestacks"
#include "llvm/CodeGen/LiveStackAnalysis.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveIntervalAnalysis.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include <limits>
using namespace llvm;

char LiveStacks::ID = 0;
INITIALIZE_PASS_BEGIN(LiveStacks, "livestacks",
                "Live Stack Slot Analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(SlotIndexes)
INITIALIZE_PASS_END(LiveStacks, "livestacks",
                "Live Stack Slot Analysis", false, false)

char &llvm::LiveStacksID = LiveStacks::ID;

void LiveStacks::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addPreserved<SlotIndexes>();
  AU.addRequiredTransitive<SlotIndexes>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

void LiveStacks::releaseMemory() {
  // Release VNInfo memory regions, VNInfo objects don't need to be dtor'd.
  VNInfoAllocator.Reset();
  S2IMap.clear();
  S2RCMap.clear();
}

bool LiveStacks::runOnMachineFunction(MachineFunction &MF) {
  TRI = MF.getTarget().getRegisterInfo();
  // FIXME: No analysis is being done right now. We are relying on the
  // register allocators to provide the information.
  return false;
}

LiveInterval &
LiveStacks::getOrCreateInterval(int Slot, const TargetRegisterClass *RC) {
  assert(Slot >= 0 && "Spill slot indice must be >= 0");
  SS2IntervalMap::iterator I = S2IMap.find(Slot);
  if (I == S2IMap.end()) {
    I = S2IMap.insert(I, std::make_pair(Slot,
            LiveInterval(TargetRegisterInfo::index2StackSlot(Slot), 0.0F)));
    S2RCMap.insert(std::make_pair(Slot, RC));
  } else {
    // Use the largest common subclass register class.
    const TargetRegisterClass *OldRC = S2RCMap[Slot];
    S2RCMap[Slot] = TRI->getCommonSubClass(OldRC, RC);
  }
  return I->second;
}

/// print - Implement the dump method.
void LiveStacks::print(raw_ostream &OS, const Module*) const {

  OS << "********** INTERVALS **********\n";
  for (const_iterator I = begin(), E = end(); I != E; ++I) {
    I->second.print(OS);
    int Slot = I->first;
    const TargetRegisterClass *RC = getIntervalRegClass(Slot);
    if (RC)
      OS << " [" << RC->getName() << "]\n";
    else
      OS << " [Unknown]\n";
  }
}