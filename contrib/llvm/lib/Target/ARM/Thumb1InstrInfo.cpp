
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

#include "Thumb1InstrInfo.h"
#include "ARM.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

Thumb1InstrInfo::Thumb1InstrInfo(const ARMSubtarget &STI)
  : ARMBaseInstrInfo(STI), RI(*this, STI) {
}

/// getNoopForMachoTarget - Return the noop instruction to use for a noop.
void Thumb1InstrInfo::getNoopForMachoTarget(MCInst &NopInst) const {
  NopInst.setOpcode(ARM::tMOVr);
  NopInst.addOperand(MCOperand::CreateReg(ARM::R8));
  NopInst.addOperand(MCOperand::CreateReg(ARM::R8));
  NopInst.addOperand(MCOperand::CreateImm(ARMCC::AL));
  NopInst.addOperand(MCOperand::CreateReg(0));
}

unsigned Thumb1InstrInfo::getUnindexedOpcode(unsigned Opc) const {
  return 0;
}

void Thumb1InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I, DebugLoc DL,
                                  unsigned DestReg, unsigned SrcReg,
                                  bool KillSrc) const {
  AddDefaultPred(BuildMI(MBB, I, DL, get(ARM::tMOVr), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc)));
  assert(ARM::GPRRegClass.contains(DestReg, SrcReg) &&
         "Thumb1 can only copy GPR registers");
}

void Thumb1InstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
  assert((RC == &ARM::tGPRRegClass ||
          (TargetRegisterInfo::isPhysicalRegister(SrcReg) &&
           isARMLowRegister(SrcReg))) && "Unknown regclass!");

  if (RC == &ARM::tGPRRegClass ||
      (TargetRegisterInfo::isPhysicalRegister(SrcReg) &&
       isARMLowRegister(SrcReg))) {
    DebugLoc DL;
    if (I != MBB.end()) DL = I->getDebugLoc();

    MachineFunction &MF = *MBB.getParent();
    MachineFrameInfo &MFI = *MF.getFrameInfo();
    MachineMemOperand *MMO =
      MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FI),
                              MachineMemOperand::MOStore,
                              MFI.getObjectSize(FI),
                              MFI.getObjectAlignment(FI));
    AddDefaultPred(BuildMI(MBB, I, DL, get(ARM::tSTRspi))
                   .addReg(SrcReg, getKillRegState(isKill))
                   .addFrameIndex(FI).addImm(0).addMemOperand(MMO));
  }
}

void Thumb1InstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int FI,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const {
  assert((RC == &ARM::tGPRRegClass ||
          (TargetRegisterInfo::isPhysicalRegister(DestReg) &&
           isARMLowRegister(DestReg))) && "Unknown regclass!");

  if (RC == &ARM::tGPRRegClass ||
      (TargetRegisterInfo::isPhysicalRegister(DestReg) &&
       isARMLowRegister(DestReg))) {
    DebugLoc DL;
    if (I != MBB.end()) DL = I->getDebugLoc();

    MachineFunction &MF = *MBB.getParent();
    MachineFrameInfo &MFI = *MF.getFrameInfo();
    MachineMemOperand *MMO =
      MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FI),
                              MachineMemOperand::MOLoad,
                              MFI.getObjectSize(FI),
                              MFI.getObjectAlignment(FI));
    AddDefaultPred(BuildMI(MBB, I, DL, get(ARM::tLDRspi), DestReg)
                   .addFrameIndex(FI).addImm(0).addMemOperand(MMO));
  }
}