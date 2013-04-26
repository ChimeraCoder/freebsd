
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

#include "Thumb2RegisterInfo.h"
#include "ARM.h"
#include "ARMBaseInstrInfo.h"
#include "ARMSubtarget.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
using namespace llvm;

Thumb2RegisterInfo::Thumb2RegisterInfo(const ARMBaseInstrInfo &tii,
                                       const ARMSubtarget &sti)
  : ARMBaseRegisterInfo(tii, sti) {
}

/// emitLoadConstPool - Emits a load from constpool to materialize the
/// specified immediate.
void
Thumb2RegisterInfo::emitLoadConstPool(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator &MBBI,
                                      DebugLoc dl,
                                      unsigned DestReg, unsigned SubIdx,
                                      int Val,
                                      ARMCC::CondCodes Pred, unsigned PredReg,
                                      unsigned MIFlags) const {
  MachineFunction &MF = *MBB.getParent();
  MachineConstantPool *ConstantPool = MF.getConstantPool();
  const Constant *C = ConstantInt::get(
           Type::getInt32Ty(MBB.getParent()->getFunction()->getContext()), Val);
  unsigned Idx = ConstantPool->getConstantPoolIndex(C, 4);

  BuildMI(MBB, MBBI, dl, TII.get(ARM::t2LDRpci))
    .addReg(DestReg, getDefRegState(true), SubIdx)
    .addConstantPoolIndex(Idx).addImm((int64_t)ARMCC::AL).addReg(0)
    .setMIFlags(MIFlags);
}