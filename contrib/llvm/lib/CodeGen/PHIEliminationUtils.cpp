
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

#include "PHIEliminationUtils.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
using namespace llvm;

// findCopyInsertPoint - Find a safe place in MBB to insert a copy from SrcReg
// when following the CFG edge to SuccMBB. This needs to be after any def of
// SrcReg, but before any subsequent point where control flow might jump out of
// the basic block.
MachineBasicBlock::iterator
llvm::findPHICopyInsertPoint(MachineBasicBlock* MBB, MachineBasicBlock* SuccMBB,
                             unsigned SrcReg) {
  // Handle the trivial case trivially.
  if (MBB->empty())
    return MBB->begin();

  // Usually, we just want to insert the copy before the first terminator
  // instruction. However, for the edge going to a landing pad, we must insert
  // the copy before the call/invoke instruction.
  if (!SuccMBB->isLandingPad())
    return MBB->getFirstTerminator();

  // Discover any defs/uses in this basic block.
  SmallPtrSet<MachineInstr*, 8> DefUsesInMBB;
  MachineRegisterInfo& MRI = MBB->getParent()->getRegInfo();
  for (MachineRegisterInfo::reg_iterator RI = MRI.reg_begin(SrcReg),
         RE = MRI.reg_end(); RI != RE; ++RI) {
    MachineInstr* DefUseMI = &*RI;
    if (DefUseMI->getParent() == MBB)
      DefUsesInMBB.insert(DefUseMI);
  }

  MachineBasicBlock::iterator InsertPoint;
  if (DefUsesInMBB.empty()) {
    // No defs.  Insert the copy at the start of the basic block.
    InsertPoint = MBB->begin();
  } else if (DefUsesInMBB.size() == 1) {
    // Insert the copy immediately after the def/use.
    InsertPoint = *DefUsesInMBB.begin();
    ++InsertPoint;
  } else {
    // Insert the copy immediately after the last def/use.
    InsertPoint = MBB->end();
    while (!DefUsesInMBB.count(&*--InsertPoint)) {}
    ++InsertPoint;
  }

  // Make sure the copy goes after any phi nodes however.
  return MBB->SkipPHIsAndLabels(InsertPoint);
}