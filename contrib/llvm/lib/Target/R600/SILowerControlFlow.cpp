
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

#include "AMDGPU.h"
#include "SIInstrInfo.h"
#include "SIMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

namespace {

class SILowerControlFlowPass : public MachineFunctionPass {

private:
  static const unsigned SkipThreshold = 12;

  static char ID;
  const TargetRegisterInfo *TRI;
  const TargetInstrInfo *TII;

  bool shouldSkip(MachineBasicBlock *From, MachineBasicBlock *To);

  void Skip(MachineInstr &From, MachineOperand &To);
  void SkipIfDead(MachineInstr &MI);

  void If(MachineInstr &MI);
  void Else(MachineInstr &MI);
  void Break(MachineInstr &MI);
  void IfBreak(MachineInstr &MI);
  void ElseBreak(MachineInstr &MI);
  void Loop(MachineInstr &MI);
  void EndCf(MachineInstr &MI);

  void Kill(MachineInstr &MI);
  void Branch(MachineInstr &MI);

  void LoadM0(MachineInstr &MI, MachineInstr *MovRel);
  void IndirectSrc(MachineInstr &MI);
  void IndirectDst(MachineInstr &MI);

public:
  SILowerControlFlowPass(TargetMachine &tm) :
    MachineFunctionPass(ID), TRI(tm.getRegisterInfo()),
    TII(tm.getInstrInfo()) { }

  virtual bool runOnMachineFunction(MachineFunction &MF);

  const char *getPassName() const {
    return "SI Lower control flow instructions";
  }

};

} // End anonymous namespace

char SILowerControlFlowPass::ID = 0;

FunctionPass *llvm::createSILowerControlFlowPass(TargetMachine &tm) {
  return new SILowerControlFlowPass(tm);
}

bool SILowerControlFlowPass::shouldSkip(MachineBasicBlock *From,
                                        MachineBasicBlock *To) {

  unsigned NumInstr = 0;

  for (MachineBasicBlock *MBB = From; MBB != To && !MBB->succ_empty();
       MBB = *MBB->succ_begin()) {

    for (MachineBasicBlock::iterator I = MBB->begin(), E = MBB->end();
         NumInstr < SkipThreshold && I != E; ++I) {

      if (I->isBundle() || !I->isBundled())
        if (++NumInstr >= SkipThreshold)
          return true;
    }
  }

  return false;
}

void SILowerControlFlowPass::Skip(MachineInstr &From, MachineOperand &To) {

  if (!shouldSkip(*From.getParent()->succ_begin(), To.getMBB()))
    return;

  DebugLoc DL = From.getDebugLoc();
  BuildMI(*From.getParent(), &From, DL, TII->get(AMDGPU::S_CBRANCH_EXECZ))
          .addOperand(To)
          .addReg(AMDGPU::EXEC);
}

void SILowerControlFlowPass::SkipIfDead(MachineInstr &MI) {

  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  if (!shouldSkip(&MBB, &MBB.getParent()->back()))
    return;

  MachineBasicBlock::iterator Insert = &MI;
  ++Insert;

  // If the exec mask is non-zero, skip the next two instructions
  BuildMI(MBB, Insert, DL, TII->get(AMDGPU::S_CBRANCH_EXECNZ))
          .addImm(3)
          .addReg(AMDGPU::EXEC);

  // Exec mask is zero: Export to NULL target...
  BuildMI(MBB, Insert, DL, TII->get(AMDGPU::EXP))
          .addImm(0)
          .addImm(0x09) // V_008DFC_SQ_EXP_NULL
          .addImm(0)
          .addImm(1)
          .addImm(1)
          .addReg(AMDGPU::VGPR0)
          .addReg(AMDGPU::VGPR0)
          .addReg(AMDGPU::VGPR0)
          .addReg(AMDGPU::VGPR0);

  // ... and terminate wavefront
  BuildMI(MBB, Insert, DL, TII->get(AMDGPU::S_ENDPGM));
}

void SILowerControlFlowPass::If(MachineInstr &MI) {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();
  unsigned Reg = MI.getOperand(0).getReg();
  unsigned Vcc = MI.getOperand(1).getReg();

  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_AND_SAVEEXEC_B64), Reg)
          .addReg(Vcc);

  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_XOR_B64), Reg)
          .addReg(AMDGPU::EXEC)
          .addReg(Reg);

  Skip(MI, MI.getOperand(2));

  MI.eraseFromParent();
}

void SILowerControlFlowPass::Else(MachineInstr &MI) {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();
  unsigned Dst = MI.getOperand(0).getReg();
  unsigned Src = MI.getOperand(1).getReg();

  BuildMI(MBB, MBB.getFirstNonPHI(), DL,
          TII->get(AMDGPU::S_OR_SAVEEXEC_B64), Dst)
          .addReg(Src); // Saved EXEC

  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_XOR_B64), AMDGPU::EXEC)
          .addReg(AMDGPU::EXEC)
          .addReg(Dst);

  Skip(MI, MI.getOperand(2));

  MI.eraseFromParent();
}

void SILowerControlFlowPass::Break(MachineInstr &MI) {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  unsigned Dst = MI.getOperand(0).getReg();
  unsigned Src = MI.getOperand(1).getReg();
 
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_OR_B64), Dst)
          .addReg(AMDGPU::EXEC)
          .addReg(Src);

  MI.eraseFromParent();
}

void SILowerControlFlowPass::IfBreak(MachineInstr &MI) {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  unsigned Dst = MI.getOperand(0).getReg();
  unsigned Vcc = MI.getOperand(1).getReg();
  unsigned Src = MI.getOperand(2).getReg();
 
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_OR_B64), Dst)
          .addReg(Vcc)
          .addReg(Src);

  MI.eraseFromParent();
}

void SILowerControlFlowPass::ElseBreak(MachineInstr &MI) {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  unsigned Dst = MI.getOperand(0).getReg();
  unsigned Saved = MI.getOperand(1).getReg();
  unsigned Src = MI.getOperand(2).getReg();
 
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_OR_B64), Dst)
          .addReg(Saved)
          .addReg(Src);

  MI.eraseFromParent();
}

void SILowerControlFlowPass::Loop(MachineInstr &MI) {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();
  unsigned Src = MI.getOperand(0).getReg();

  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_ANDN2_B64), AMDGPU::EXEC)
          .addReg(AMDGPU::EXEC)
          .addReg(Src);

  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_CBRANCH_EXECNZ))
          .addOperand(MI.getOperand(1))
          .addReg(AMDGPU::EXEC);

  MI.eraseFromParent();
}

void SILowerControlFlowPass::EndCf(MachineInstr &MI) {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();
  unsigned Reg = MI.getOperand(0).getReg();

  BuildMI(MBB, MBB.getFirstNonPHI(), DL,
          TII->get(AMDGPU::S_OR_B64), AMDGPU::EXEC)
          .addReg(AMDGPU::EXEC)
          .addReg(Reg);

  MI.eraseFromParent();
}

void SILowerControlFlowPass::Branch(MachineInstr &MI) {
  MachineBasicBlock *Next = MI.getParent()->getNextNode();
  MachineBasicBlock *Target = MI.getOperand(0).getMBB();
  if (Target == Next)
    MI.eraseFromParent();
  else
    assert(0);
}

void SILowerControlFlowPass::Kill(MachineInstr &MI) {

  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  // Kill is only allowed in pixel shaders
  assert(MBB.getParent()->getInfo<SIMachineFunctionInfo>()->ShaderType ==
         ShaderType::PIXEL);

  // Clear this pixel from the exec mask if the operand is negative
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::V_CMPX_LE_F32_e32), AMDGPU::VCC)
          .addImm(0)
          .addOperand(MI.getOperand(0));

  MI.eraseFromParent();
}

void SILowerControlFlowPass::LoadM0(MachineInstr &MI, MachineInstr *MovRel) {

  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();
  MachineBasicBlock::iterator I = MI;

  unsigned Save = MI.getOperand(1).getReg();
  unsigned Idx = MI.getOperand(3).getReg();

  if (AMDGPU::SReg_32RegClass.contains(Idx)) {
    BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_MOV_B32), AMDGPU::M0)
            .addReg(Idx);
    MBB.insert(I, MovRel);
    MI.eraseFromParent();
    return;
  }

  assert(AMDGPU::SReg_64RegClass.contains(Save));
  assert(AMDGPU::VReg_32RegClass.contains(Idx));

  // Save the EXEC mask
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_MOV_B64), Save)
          .addReg(AMDGPU::EXEC);

  // Read the next variant into VCC (lower 32 bits) <- also loop target
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::V_READFIRSTLANE_B32_e32), AMDGPU::VCC)
          .addReg(Idx);

  // Move index from VCC into M0
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_MOV_B32), AMDGPU::M0)
          .addReg(AMDGPU::VCC);

  // Compare the just read M0 value to all possible Idx values
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::V_CMP_EQ_U32_e32), AMDGPU::VCC)
          .addReg(AMDGPU::M0)
          .addReg(Idx);

  // Update EXEC, save the original EXEC value to VCC
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_AND_SAVEEXEC_B64), AMDGPU::VCC)
          .addReg(AMDGPU::VCC);

  // Do the actual move
  MBB.insert(I, MovRel);

  // Update EXEC, switch all done bits to 0 and all todo bits to 1
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_XOR_B64), AMDGPU::EXEC)
          .addReg(AMDGPU::EXEC)
          .addReg(AMDGPU::VCC);

  // Loop back to V_READFIRSTLANE_B32 if there are still variants to cover
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_CBRANCH_EXECNZ))
          .addImm(-7)
          .addReg(AMDGPU::EXEC);

  // Restore EXEC
  BuildMI(MBB, &MI, DL, TII->get(AMDGPU::S_MOV_B64), AMDGPU::EXEC)
          .addReg(Save);

  MI.eraseFromParent();
}

void SILowerControlFlowPass::IndirectSrc(MachineInstr &MI) {

  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  unsigned Dst = MI.getOperand(0).getReg();
  unsigned Vec = MI.getOperand(2).getReg();
  unsigned Off = MI.getOperand(4).getImm();

  MachineInstr *MovRel = 
    BuildMI(*MBB.getParent(), DL, TII->get(AMDGPU::V_MOVRELS_B32_e32), Dst)
            .addReg(TRI->getSubReg(Vec, AMDGPU::sub0) + Off)
            .addReg(AMDGPU::M0, RegState::Implicit)
            .addReg(Vec, RegState::Implicit);

  LoadM0(MI, MovRel);
}

void SILowerControlFlowPass::IndirectDst(MachineInstr &MI) {

  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  unsigned Dst = MI.getOperand(0).getReg();
  unsigned Off = MI.getOperand(4).getImm();
  unsigned Val = MI.getOperand(5).getReg();

  MachineInstr *MovRel = 
    BuildMI(*MBB.getParent(), DL, TII->get(AMDGPU::V_MOVRELD_B32_e32))
            .addReg(TRI->getSubReg(Dst, AMDGPU::sub0) + Off, RegState::Define)
            .addReg(Val)
            .addReg(AMDGPU::M0, RegState::Implicit)
            .addReg(Dst, RegState::Implicit);

  LoadM0(MI, MovRel);
}

bool SILowerControlFlowPass::runOnMachineFunction(MachineFunction &MF) {

  bool HaveKill = false;
  bool NeedWQM = false;
  unsigned Depth = 0;

  for (MachineFunction::iterator BI = MF.begin(), BE = MF.end();
       BI != BE; ++BI) {

    MachineBasicBlock &MBB = *BI;
    for (MachineBasicBlock::iterator I = MBB.begin(), Next = llvm::next(I);
         I != MBB.end(); I = Next) {

      Next = llvm::next(I);
      MachineInstr &MI = *I;
      switch (MI.getOpcode()) {
        default: break;
        case AMDGPU::SI_IF:
          ++Depth;
          If(MI);
          break;

        case AMDGPU::SI_ELSE:
          Else(MI);
          break;

        case AMDGPU::SI_BREAK:
          Break(MI);
          break;

        case AMDGPU::SI_IF_BREAK:
          IfBreak(MI);
          break;

        case AMDGPU::SI_ELSE_BREAK:
          ElseBreak(MI);
          break;

        case AMDGPU::SI_LOOP:
          ++Depth;
          Loop(MI);
          break;

        case AMDGPU::SI_END_CF:
          if (--Depth == 0 && HaveKill) {
            SkipIfDead(MI);
            HaveKill = false;
          }
          EndCf(MI);
          break;

        case AMDGPU::SI_KILL:
          if (Depth == 0)
            SkipIfDead(MI);
          else
            HaveKill = true;
          Kill(MI);
          break;

        case AMDGPU::S_BRANCH:
          Branch(MI);
          break;

        case AMDGPU::SI_INDIRECT_SRC:
          IndirectSrc(MI);
          break;

        case AMDGPU::SI_INDIRECT_DST_V2:
        case AMDGPU::SI_INDIRECT_DST_V4:
        case AMDGPU::SI_INDIRECT_DST_V8:
        case AMDGPU::SI_INDIRECT_DST_V16:
          IndirectDst(MI);
          break;

        case AMDGPU::V_INTERP_P1_F32:
        case AMDGPU::V_INTERP_P2_F32:
        case AMDGPU::V_INTERP_MOV_F32:
          NeedWQM = true;
          break;

      }
    }
  }

  if (NeedWQM) {
    MachineBasicBlock &MBB = MF.front();
    BuildMI(MBB, MBB.getFirstNonPHI(), DebugLoc(), TII->get(AMDGPU::S_WQM_B64),
            AMDGPU::EXEC).addReg(AMDGPU::EXEC);
  }

  return true;
}