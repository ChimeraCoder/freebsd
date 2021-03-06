
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

#define DEBUG_TYPE "regalloc"
#include "llvm/CodeGen/RegisterClassInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

static cl::opt<unsigned>
StressRA("stress-regalloc", cl::Hidden, cl::init(0), cl::value_desc("N"),
         cl::desc("Limit all regclasses to N registers"));

RegisterClassInfo::RegisterClassInfo() : Tag(0), MF(0), TRI(0), CalleeSaved(0)
{}

void RegisterClassInfo::runOnMachineFunction(const MachineFunction &mf) {
  bool Update = false;
  MF = &mf;

  // Allocate new array the first time we see a new target.
  if (MF->getTarget().getRegisterInfo() != TRI) {
    TRI = MF->getTarget().getRegisterInfo();
    RegClass.reset(new RCInfo[TRI->getNumRegClasses()]);
    Update = true;
  }

  // Does this MF have different CSRs?
  const MCPhysReg *CSR = TRI->getCalleeSavedRegs(MF);
  if (Update || CSR != CalleeSaved) {
    // Build a CSRNum map. Every CSR alias gets an entry pointing to the last
    // overlapping CSR.
    CSRNum.clear();
    CSRNum.resize(TRI->getNumRegs(), 0);
    for (unsigned N = 0; unsigned Reg = CSR[N]; ++N)
      for (MCRegAliasIterator AI(Reg, TRI, true); AI.isValid(); ++AI)
        CSRNum[*AI] = N + 1; // 0 means no CSR, 1 means CalleeSaved[0], ...
    Update = true;
  }
  CalleeSaved = CSR;

  // Different reserved registers?
  const BitVector &RR = MF->getRegInfo().getReservedRegs();
  if (Reserved.size() != RR.size() || RR != Reserved) {
    Update = true;
    Reserved = RR;
  }

  // Invalidate cached information from previous function.
  if (Update)
    ++Tag;
}

/// compute - Compute the preferred allocation order for RC with reserved
/// registers filtered out. Volatile registers come first followed by CSR
/// aliases ordered according to the CSR order specified by the target.
void RegisterClassInfo::compute(const TargetRegisterClass *RC) const {
  RCInfo &RCI = RegClass[RC->getID()];

  // Raw register count, including all reserved regs.
  unsigned NumRegs = RC->getNumRegs();

  if (!RCI.Order)
    RCI.Order.reset(new MCPhysReg[NumRegs]);

  unsigned N = 0;
  SmallVector<MCPhysReg, 16> CSRAlias;
  unsigned MinCost = 0xff;
  unsigned LastCost = ~0u;
  unsigned LastCostChange = 0;

  // FIXME: Once targets reserve registers instead of removing them from the
  // allocation order, we can simply use begin/end here.
  ArrayRef<MCPhysReg> RawOrder = RC->getRawAllocationOrder(*MF);
  for (unsigned i = 0; i != RawOrder.size(); ++i) {
    unsigned PhysReg = RawOrder[i];
    // Remove reserved registers from the allocation order.
    if (Reserved.test(PhysReg))
      continue;
    unsigned Cost = TRI->getCostPerUse(PhysReg);
    MinCost = std::min(MinCost, Cost);

    if (CSRNum[PhysReg])
      // PhysReg aliases a CSR, save it for later.
      CSRAlias.push_back(PhysReg);
    else {
      if (Cost != LastCost)
        LastCostChange = N;
      RCI.Order[N++] = PhysReg;
      LastCost = Cost;
    }
  }
  RCI.NumRegs = N + CSRAlias.size();
  assert (RCI.NumRegs <= NumRegs && "Allocation order larger than regclass");

  // CSR aliases go after the volatile registers, preserve the target's order.
  for (unsigned i = 0, e = CSRAlias.size(); i != e; ++i) {
    unsigned PhysReg = CSRAlias[i];
    unsigned Cost = TRI->getCostPerUse(PhysReg);
    if (Cost != LastCost)
      LastCostChange = N;
    RCI.Order[N++] = PhysReg;
    LastCost = Cost;
  }

  // Register allocator stress test.  Clip register class to N registers.
  if (StressRA && RCI.NumRegs > StressRA)
    RCI.NumRegs = StressRA;

  // Check if RC is a proper sub-class.
  if (const TargetRegisterClass *Super = TRI->getLargestLegalSuperClass(RC))
    if (Super != RC && getNumAllocatableRegs(Super) > RCI.NumRegs)
      RCI.ProperSubClass = true;

  RCI.MinCost = uint8_t(MinCost);
  RCI.LastCostChange = LastCostChange;

  DEBUG({
    dbgs() << "AllocationOrder(" << RC->getName() << ") = [";
    for (unsigned I = 0; I != RCI.NumRegs; ++I)
      dbgs() << ' ' << PrintReg(RCI.Order[I], TRI);
    dbgs() << (RCI.ProperSubClass ? " ] (sub-class)\n" : " ]\n");
  });

  // RCI is now up-to-date.
  RCI.Tag = Tag;
}