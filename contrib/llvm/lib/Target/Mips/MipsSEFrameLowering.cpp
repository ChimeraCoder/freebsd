
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

#include "MipsSEFrameLowering.h"
#include "MCTargetDesc/MipsBaseInfo.h"
#include "MipsAnalyzeImmediate.h"
#include "MipsMachineFunction.h"
#include "MipsSEInstrInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

namespace {
typedef MachineBasicBlock::iterator Iter;

/// Helper class to expand accumulator pseudos.
class ExpandACCPseudo {
public:
  ExpandACCPseudo(MachineFunction &MF);
  bool expand();

private:
  bool expandInstr(MachineBasicBlock &MBB, Iter I);
  void expandLoad(MachineBasicBlock &MBB, Iter I, unsigned RegSize);
  void expandStore(MachineBasicBlock &MBB, Iter I, unsigned RegSize);
  void expandCopy(MachineBasicBlock &MBB, Iter I, unsigned RegSize);

  MachineFunction &MF;
  const MipsSEInstrInfo &TII;
  const MipsRegisterInfo &RegInfo;
  MachineRegisterInfo &MRI;
};
}

ExpandACCPseudo::ExpandACCPseudo(MachineFunction &MF_)
  : MF(MF_),
    TII(*static_cast<const MipsSEInstrInfo*>(MF.getTarget().getInstrInfo())),
    RegInfo(TII.getRegisterInfo()), MRI(MF.getRegInfo()) {}

bool ExpandACCPseudo::expand() {
  bool Expanded = false;

  for (MachineFunction::iterator BB = MF.begin(), BBEnd = MF.end();
       BB != BBEnd; ++BB)
    for (Iter I = BB->begin(), End = BB->end(); I != End;)
      Expanded |= expandInstr(*BB, I++);

  return Expanded;
}

bool ExpandACCPseudo::expandInstr(MachineBasicBlock &MBB, Iter I) {
  switch(I->getOpcode()) {
  case Mips::LOAD_AC64:
  case Mips::LOAD_AC64_P8:
  case Mips::LOAD_AC_DSP:
  case Mips::LOAD_AC_DSP_P8:
    expandLoad(MBB, I, 4);
    break;
  case Mips::LOAD_AC128:
  case Mips::LOAD_AC128_P8:
    expandLoad(MBB, I, 8);
    break;
  case Mips::STORE_AC64:
  case Mips::STORE_AC64_P8:
  case Mips::STORE_AC_DSP:
  case Mips::STORE_AC_DSP_P8:
    expandStore(MBB, I, 4);
    break;
  case Mips::STORE_AC128:
  case Mips::STORE_AC128_P8:
    expandStore(MBB, I, 8);
    break;
  case Mips::COPY_AC64:
  case Mips::COPY_AC_DSP:
    expandCopy(MBB, I, 4);
    break;
  case Mips::COPY_AC128:
    expandCopy(MBB, I, 8);
    break;
  default:
    return false;
  }

  MBB.erase(I);
  return true;
}

void ExpandACCPseudo::expandLoad(MachineBasicBlock &MBB, Iter I,
                                 unsigned RegSize) {
  //  load $vr0, FI
  //  copy lo, $vr0
  //  load $vr1, FI + 4
  //  copy hi, $vr1

  assert(I->getOperand(0).isReg() && I->getOperand(1).isFI());

  const TargetRegisterClass *RC = RegInfo.intRegClass(RegSize);
  unsigned VR0 = MRI.createVirtualRegister(RC);
  unsigned VR1 = MRI.createVirtualRegister(RC);
  unsigned Dst = I->getOperand(0).getReg(), FI = I->getOperand(1).getIndex();
  unsigned Lo = RegInfo.getSubReg(Dst, Mips::sub_lo);
  unsigned Hi = RegInfo.getSubReg(Dst, Mips::sub_hi);
  DebugLoc DL = I->getDebugLoc();
  const MCInstrDesc &Desc = TII.get(TargetOpcode::COPY);

  TII.loadRegFromStack(MBB, I, VR0, FI, RC, &RegInfo, 0);
  BuildMI(MBB, I, DL, Desc, Lo).addReg(VR0, RegState::Kill);
  TII.loadRegFromStack(MBB, I, VR1, FI, RC, &RegInfo, RegSize);
  BuildMI(MBB, I, DL, Desc, Hi).addReg(VR1, RegState::Kill);
}

void ExpandACCPseudo::expandStore(MachineBasicBlock &MBB, Iter I,
                                  unsigned RegSize) {
  //  copy $vr0, lo
  //  store $vr0, FI
  //  copy $vr1, hi
  //  store $vr1, FI + 4

  assert(I->getOperand(0).isReg() && I->getOperand(1).isFI());

  const TargetRegisterClass *RC = RegInfo.intRegClass(RegSize);
  unsigned VR0 = MRI.createVirtualRegister(RC);
  unsigned VR1 = MRI.createVirtualRegister(RC);
  unsigned Src = I->getOperand(0).getReg(), FI = I->getOperand(1).getIndex();
  unsigned SrcKill = getKillRegState(I->getOperand(0).isKill());
  unsigned Lo = RegInfo.getSubReg(Src, Mips::sub_lo);
  unsigned Hi = RegInfo.getSubReg(Src, Mips::sub_hi);
  DebugLoc DL = I->getDebugLoc();

  BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), VR0).addReg(Lo, SrcKill);
  TII.storeRegToStack(MBB, I, VR0, true, FI, RC, &RegInfo, 0);
  BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), VR1).addReg(Hi, SrcKill);
  TII.storeRegToStack(MBB, I, VR1, true, FI, RC, &RegInfo, RegSize);
}

void ExpandACCPseudo::expandCopy(MachineBasicBlock &MBB, Iter I,
                                 unsigned RegSize) {
  //  copy $vr0, src_lo
  //  copy dst_lo, $vr0
  //  copy $vr1, src_hi
  //  copy dst_hi, $vr1

  const TargetRegisterClass *RC = RegInfo.intRegClass(RegSize);
  unsigned VR0 = MRI.createVirtualRegister(RC);
  unsigned VR1 = MRI.createVirtualRegister(RC);
  unsigned Dst = I->getOperand(0).getReg(), Src = I->getOperand(1).getReg();
  unsigned SrcKill = getKillRegState(I->getOperand(1).isKill());
  unsigned DstLo = RegInfo.getSubReg(Dst, Mips::sub_lo);
  unsigned DstHi = RegInfo.getSubReg(Dst, Mips::sub_hi);
  unsigned SrcLo = RegInfo.getSubReg(Src, Mips::sub_lo);
  unsigned SrcHi = RegInfo.getSubReg(Src, Mips::sub_hi);
  DebugLoc DL = I->getDebugLoc();

  BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), VR0).addReg(SrcLo, SrcKill);
  BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), DstLo)
    .addReg(VR0, RegState::Kill);
  BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), VR1).addReg(SrcHi, SrcKill);
  BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), DstHi)
    .addReg(VR1, RegState::Kill);
}

unsigned MipsSEFrameLowering::ehDataReg(unsigned I) const {
  static const unsigned EhDataReg[] = {
    Mips::A0, Mips::A1, Mips::A2, Mips::A3
  };
  static const unsigned EhDataReg64[] = {
    Mips::A0_64, Mips::A1_64, Mips::A2_64, Mips::A3_64
  };

  return STI.isABI_N64() ? EhDataReg64[I] : EhDataReg[I];
}

void MipsSEFrameLowering::emitPrologue(MachineFunction &MF) const {
  MachineBasicBlock &MBB   = MF.front();
  MachineFrameInfo *MFI    = MF.getFrameInfo();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();
  const MipsRegisterInfo *RegInfo =
    static_cast<const MipsRegisterInfo*>(MF.getTarget().getRegisterInfo());
  const MipsSEInstrInfo &TII =
    *static_cast<const MipsSEInstrInfo*>(MF.getTarget().getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  unsigned SP = STI.isABI_N64() ? Mips::SP_64 : Mips::SP;
  unsigned FP = STI.isABI_N64() ? Mips::FP_64 : Mips::FP;
  unsigned ZERO = STI.isABI_N64() ? Mips::ZERO_64 : Mips::ZERO;
  unsigned ADDu = STI.isABI_N64() ? Mips::DADDu : Mips::ADDu;

  // First, compute final stack size.
  uint64_t StackSize = MFI->getStackSize();

  // No need to allocate space on the stack.
  if (StackSize == 0 && !MFI->adjustsStack()) return;

  MachineModuleInfo &MMI = MF.getMMI();
  std::vector<MachineMove> &Moves = MMI.getFrameMoves();
  MachineLocation DstML, SrcML;

  // Adjust stack.
  TII.adjustStackPtr(SP, -StackSize, MBB, MBBI);

  // emit ".cfi_def_cfa_offset StackSize"
  MCSymbol *AdjustSPLabel = MMI.getContext().CreateTempSymbol();
  BuildMI(MBB, MBBI, dl,
          TII.get(TargetOpcode::PROLOG_LABEL)).addSym(AdjustSPLabel);
  DstML = MachineLocation(MachineLocation::VirtualFP);
  SrcML = MachineLocation(MachineLocation::VirtualFP, -StackSize);
  Moves.push_back(MachineMove(AdjustSPLabel, DstML, SrcML));

  const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();

  if (CSI.size()) {
    // Find the instruction past the last instruction that saves a callee-saved
    // register to the stack.
    for (unsigned i = 0; i < CSI.size(); ++i)
      ++MBBI;

    // Iterate over list of callee-saved registers and emit .cfi_offset
    // directives.
    MCSymbol *CSLabel = MMI.getContext().CreateTempSymbol();
    BuildMI(MBB, MBBI, dl,
            TII.get(TargetOpcode::PROLOG_LABEL)).addSym(CSLabel);

    for (std::vector<CalleeSavedInfo>::const_iterator I = CSI.begin(),
           E = CSI.end(); I != E; ++I) {
      int64_t Offset = MFI->getObjectOffset(I->getFrameIdx());
      unsigned Reg = I->getReg();

      // If Reg is a double precision register, emit two cfa_offsets,
      // one for each of the paired single precision registers.
      if (Mips::AFGR64RegClass.contains(Reg)) {
        MachineLocation DstML0(MachineLocation::VirtualFP, Offset);
        MachineLocation DstML1(MachineLocation::VirtualFP, Offset + 4);
        MachineLocation SrcML0(RegInfo->getSubReg(Reg, Mips::sub_fpeven));
        MachineLocation SrcML1(RegInfo->getSubReg(Reg, Mips::sub_fpodd));

        if (!STI.isLittle())
          std::swap(SrcML0, SrcML1);

        Moves.push_back(MachineMove(CSLabel, DstML0, SrcML0));
        Moves.push_back(MachineMove(CSLabel, DstML1, SrcML1));
      } else {
        // Reg is either in CPURegs or FGR32.
        DstML = MachineLocation(MachineLocation::VirtualFP, Offset);
        SrcML = MachineLocation(Reg);
        Moves.push_back(MachineMove(CSLabel, DstML, SrcML));
      }
    }
  }

  if (MipsFI->callsEhReturn()) {
    const TargetRegisterClass *RC = STI.isABI_N64() ?
        &Mips::CPU64RegsRegClass : &Mips::CPURegsRegClass;

    // Insert instructions that spill eh data registers.
    for (int I = 0; I < 4; ++I) {
      if (!MBB.isLiveIn(ehDataReg(I)))
        MBB.addLiveIn(ehDataReg(I));
      TII.storeRegToStackSlot(MBB, MBBI, ehDataReg(I), false,
                              MipsFI->getEhDataRegFI(I), RC, RegInfo);
    }

    // Emit .cfi_offset directives for eh data registers.
    MCSymbol *CSLabel2 = MMI.getContext().CreateTempSymbol();
    BuildMI(MBB, MBBI, dl,
            TII.get(TargetOpcode::PROLOG_LABEL)).addSym(CSLabel2);
    for (int I = 0; I < 4; ++I) {
      int64_t Offset = MFI->getObjectOffset(MipsFI->getEhDataRegFI(I));
      DstML = MachineLocation(MachineLocation::VirtualFP, Offset);
      SrcML = MachineLocation(ehDataReg(I));
      Moves.push_back(MachineMove(CSLabel2, DstML, SrcML));
    }
  }

  // if framepointer enabled, set it to point to the stack pointer.
  if (hasFP(MF)) {
    // Insert instruction "move $fp, $sp" at this location.
    BuildMI(MBB, MBBI, dl, TII.get(ADDu), FP).addReg(SP).addReg(ZERO);

    // emit ".cfi_def_cfa_register $fp"
    MCSymbol *SetFPLabel = MMI.getContext().CreateTempSymbol();
    BuildMI(MBB, MBBI, dl,
            TII.get(TargetOpcode::PROLOG_LABEL)).addSym(SetFPLabel);
    DstML = MachineLocation(FP);
    SrcML = MachineLocation(MachineLocation::VirtualFP);
    Moves.push_back(MachineMove(SetFPLabel, DstML, SrcML));
  }
}

void MipsSEFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo *MFI            = MF.getFrameInfo();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();
  const MipsRegisterInfo *RegInfo =
    static_cast<const MipsRegisterInfo*>(MF.getTarget().getRegisterInfo());
  const MipsSEInstrInfo &TII =
    *static_cast<const MipsSEInstrInfo*>(MF.getTarget().getInstrInfo());
  DebugLoc dl = MBBI->getDebugLoc();
  unsigned SP = STI.isABI_N64() ? Mips::SP_64 : Mips::SP;
  unsigned FP = STI.isABI_N64() ? Mips::FP_64 : Mips::FP;
  unsigned ZERO = STI.isABI_N64() ? Mips::ZERO_64 : Mips::ZERO;
  unsigned ADDu = STI.isABI_N64() ? Mips::DADDu : Mips::ADDu;

  // if framepointer enabled, restore the stack pointer.
  if (hasFP(MF)) {
    // Find the first instruction that restores a callee-saved register.
    MachineBasicBlock::iterator I = MBBI;

    for (unsigned i = 0; i < MFI->getCalleeSavedInfo().size(); ++i)
      --I;

    // Insert instruction "move $sp, $fp" at this location.
    BuildMI(MBB, I, dl, TII.get(ADDu), SP).addReg(FP).addReg(ZERO);
  }

  if (MipsFI->callsEhReturn()) {
    const TargetRegisterClass *RC = STI.isABI_N64() ?
        &Mips::CPU64RegsRegClass : &Mips::CPURegsRegClass;

    // Find first instruction that restores a callee-saved register.
    MachineBasicBlock::iterator I = MBBI;
    for (unsigned i = 0; i < MFI->getCalleeSavedInfo().size(); ++i)
      --I;

    // Insert instructions that restore eh data registers.
    for (int J = 0; J < 4; ++J) {
      TII.loadRegFromStackSlot(MBB, I, ehDataReg(J), MipsFI->getEhDataRegFI(J),
                               RC, RegInfo);
    }
  }

  // Get the number of bytes from FrameInfo
  uint64_t StackSize = MFI->getStackSize();

  if (!StackSize)
    return;

  // Adjust stack.
  TII.adjustStackPtr(SP, StackSize, MBB, MBBI);
}

bool MipsSEFrameLowering::
spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator MI,
                          const std::vector<CalleeSavedInfo> &CSI,
                          const TargetRegisterInfo *TRI) const {
  MachineFunction *MF = MBB.getParent();
  MachineBasicBlock *EntryBlock = MF->begin();
  const TargetInstrInfo &TII = *MF->getTarget().getInstrInfo();

  for (unsigned i = 0, e = CSI.size(); i != e; ++i) {
    // Add the callee-saved register as live-in. Do not add if the register is
    // RA and return address is taken, because it has already been added in
    // method MipsTargetLowering::LowerRETURNADDR.
    // It's killed at the spill, unless the register is RA and return address
    // is taken.
    unsigned Reg = CSI[i].getReg();
    bool IsRAAndRetAddrIsTaken = (Reg == Mips::RA || Reg == Mips::RA_64)
        && MF->getFrameInfo()->isReturnAddressTaken();
    if (!IsRAAndRetAddrIsTaken)
      EntryBlock->addLiveIn(Reg);

    // Insert the spill to the stack frame.
    bool IsKill = !IsRAAndRetAddrIsTaken;
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.storeRegToStackSlot(*EntryBlock, MI, Reg, IsKill,
                            CSI[i].getFrameIdx(), RC, TRI);
  }

  return true;
}

bool
MipsSEFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();

  // Reserve call frame if the size of the maximum call frame fits into 16-bit
  // immediate field and there are no variable sized objects on the stack.
  // Make sure the second register scavenger spill slot can be accessed with one
  // instruction.
  return isInt<16>(MFI->getMaxCallFrameSize() + getStackAlignment()) &&
    !MFI->hasVarSizedObjects();
}

// Eliminate ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo instructions
void MipsSEFrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  const MipsSEInstrInfo &TII =
    *static_cast<const MipsSEInstrInfo*>(MF.getTarget().getInstrInfo());

  if (!hasReservedCallFrame(MF)) {
    int64_t Amount = I->getOperand(0).getImm();

    if (I->getOpcode() == Mips::ADJCALLSTACKDOWN)
      Amount = -Amount;

    unsigned SP = STI.isABI_N64() ? Mips::SP_64 : Mips::SP;
    TII.adjustStackPtr(SP, Amount, MBB, I);
  }

  MBB.erase(I);
}

void MipsSEFrameLowering::
processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                     RegScavenger *RS) const {
  MachineRegisterInfo &MRI = MF.getRegInfo();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();
  unsigned FP = STI.isABI_N64() ? Mips::FP_64 : Mips::FP;

  // Mark $fp as used if function has dedicated frame pointer.
  if (hasFP(MF))
    MRI.setPhysRegUsed(FP);

  // Create spill slots for eh data registers if function calls eh_return.
  if (MipsFI->callsEhReturn())
    MipsFI->createEhDataRegsFI();

  // Expand pseudo instructions which load, store or copy accumulators.
  // Add an emergency spill slot if a pseudo was expanded.
  if (ExpandACCPseudo(MF).expand()) {
    // The spill slot should be half the size of the accumulator. If target is
    // mips64, it should be 64-bit, otherwise it should be 32-bt.
    const TargetRegisterClass *RC = STI.hasMips64() ?
      &Mips::CPU64RegsRegClass : &Mips::CPURegsRegClass;
    int FI = MF.getFrameInfo()->CreateStackObject(RC->getSize(),
                                                  RC->getAlignment(), false);
    RS->addScavengingFrameIndex(FI);
  }

  // Set scavenging frame index if necessary.
  uint64_t MaxSPOffset = MF.getInfo<MipsFunctionInfo>()->getIncomingArgSize() +
    estimateStackSize(MF);

  if (isInt<16>(MaxSPOffset))
    return;

  const TargetRegisterClass *RC = STI.isABI_N64() ?
    &Mips::CPU64RegsRegClass : &Mips::CPURegsRegClass;
  int FI = MF.getFrameInfo()->CreateStackObject(RC->getSize(),
                                                RC->getAlignment(), false);
  RS->addScavengingFrameIndex(FI);
}

const MipsFrameLowering *
llvm::createMipsSEFrameLowering(const MipsSubtarget &ST) {
  return new MipsSEFrameLowering(ST);
}