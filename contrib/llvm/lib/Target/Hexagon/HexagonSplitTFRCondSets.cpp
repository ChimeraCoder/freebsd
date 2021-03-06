
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

#define DEBUG_TYPE "xfer"
#include "Hexagon.h"
#include "HexagonMachineFunctionInfo.h"
#include "HexagonSubtarget.h"
#include "HexagonTargetMachine.h"
#include "llvm/CodeGen/LatencyPriorityQueue.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegisterInfo.h"

using namespace llvm;

namespace {

class HexagonSplitTFRCondSets : public MachineFunctionPass {
    HexagonTargetMachine& QTM;
    const HexagonSubtarget &QST;

 public:
    static char ID;
    HexagonSplitTFRCondSets(HexagonTargetMachine& TM) :
      MachineFunctionPass(ID), QTM(TM), QST(*TM.getSubtargetImpl()) {}

    const char *getPassName() const {
      return "Hexagon Split TFRCondSets";
    }
    bool runOnMachineFunction(MachineFunction &Fn);
};


char HexagonSplitTFRCondSets::ID = 0;


bool HexagonSplitTFRCondSets::runOnMachineFunction(MachineFunction &Fn) {

  const TargetInstrInfo *TII = QTM.getInstrInfo();

  // Loop over all of the basic blocks.
  for (MachineFunction::iterator MBBb = Fn.begin(), MBBe = Fn.end();
       MBBb != MBBe; ++MBBb) {
    MachineBasicBlock* MBB = MBBb;
    // Traverse the basic block.
    for (MachineBasicBlock::iterator MII = MBB->begin(); MII != MBB->end();
         ++MII) {
      MachineInstr *MI = MII;
      int Opc1, Opc2;
      switch(MI->getOpcode()) {
        case Hexagon::TFR_condset_rr:
        case Hexagon::TFR_condset_rr_f:
        case Hexagon::TFR_condset_rr64_f: {
          int DestReg = MI->getOperand(0).getReg();
          int SrcReg1 = MI->getOperand(2).getReg();
          int SrcReg2 = MI->getOperand(3).getReg();

          if (MI->getOpcode() == Hexagon::TFR_condset_rr ||
              MI->getOpcode() == Hexagon::TFR_condset_rr_f) {
            Opc1 = Hexagon::TFR_cPt;
            Opc2 = Hexagon::TFR_cNotPt;
          }
          else if (MI->getOpcode() == Hexagon::TFR_condset_rr64_f) {
            Opc1 = Hexagon::TFR64_cPt;
            Opc2 = Hexagon::TFR64_cNotPt;
          }

          // Minor optimization: do not emit the predicated copy if the source
          // and the destination is the same register.
          if (DestReg != SrcReg1) {
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Opc1),
                    DestReg).addReg(MI->getOperand(1).getReg()).addReg(SrcReg1);
          }
          if (DestReg != SrcReg2) {
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Opc2),
                    DestReg).addReg(MI->getOperand(1).getReg()).addReg(SrcReg2);
          }
          MII = MBB->erase(MI);
          --MII;
          break;
        }
        case Hexagon::TFR_condset_ri:
        case Hexagon::TFR_condset_ri_f: {
          int DestReg = MI->getOperand(0).getReg();
          int SrcReg1 = MI->getOperand(2).getReg();

          //  Do not emit the predicated copy if the source and the destination
          // is the same register.
          if (DestReg != SrcReg1) {
            BuildMI(*MBB, MII, MI->getDebugLoc(),
              TII->get(Hexagon::TFR_cPt), DestReg).
              addReg(MI->getOperand(1).getReg()).addReg(SrcReg1);
          }
          if (MI->getOpcode() ==  Hexagon::TFR_condset_ri ) {
            BuildMI(*MBB, MII, MI->getDebugLoc(),
              TII->get(Hexagon::TFRI_cNotPt), DestReg).
              addReg(MI->getOperand(1).getReg()).
              addImm(MI->getOperand(3).getImm());
          } else if (MI->getOpcode() ==  Hexagon::TFR_condset_ri_f ) {
            BuildMI(*MBB, MII, MI->getDebugLoc(),
              TII->get(Hexagon::TFRI_cNotPt_f), DestReg).
              addReg(MI->getOperand(1).getReg()).
              addFPImm(MI->getOperand(3).getFPImm());
          }

          MII = MBB->erase(MI);
          --MII;
          break;
        }
        case Hexagon::TFR_condset_ir:
        case Hexagon::TFR_condset_ir_f: {
          int DestReg = MI->getOperand(0).getReg();
          int SrcReg2 = MI->getOperand(3).getReg();

          if (MI->getOpcode() ==  Hexagon::TFR_condset_ir ) {
            BuildMI(*MBB, MII, MI->getDebugLoc(),
              TII->get(Hexagon::TFRI_cPt), DestReg).
              addReg(MI->getOperand(1).getReg()).
              addImm(MI->getOperand(2).getImm());
          } else if (MI->getOpcode() ==  Hexagon::TFR_condset_ir_f ) {
            BuildMI(*MBB, MII, MI->getDebugLoc(),
              TII->get(Hexagon::TFRI_cPt_f), DestReg).
              addReg(MI->getOperand(1).getReg()).
              addFPImm(MI->getOperand(2).getFPImm());
          }

          // Do not emit the predicated copy if the source and
          // the destination is the same register.
          if (DestReg != SrcReg2) {
            BuildMI(*MBB, MII, MI->getDebugLoc(),
              TII->get(Hexagon::TFR_cNotPt), DestReg).
              addReg(MI->getOperand(1).getReg()).addReg(SrcReg2);
          }
          MII = MBB->erase(MI);
          --MII;
          break;
        }
        case Hexagon::TFR_condset_ii:
        case Hexagon::TFR_condset_ii_f: {
          int DestReg = MI->getOperand(0).getReg();
          int SrcReg1 = MI->getOperand(1).getReg();

          if (MI->getOpcode() ==  Hexagon::TFR_condset_ii ) {
            int Immed1 = MI->getOperand(2).getImm();
            int Immed2 = MI->getOperand(3).getImm();
            BuildMI(*MBB, MII, MI->getDebugLoc(),
                    TII->get(Hexagon::TFRI_cPt),
                    DestReg).addReg(SrcReg1).addImm(Immed1);
            BuildMI(*MBB, MII, MI->getDebugLoc(),
                    TII->get(Hexagon::TFRI_cNotPt),
                    DestReg).addReg(SrcReg1).addImm(Immed2);
          } else if (MI->getOpcode() ==  Hexagon::TFR_condset_ii_f ) {
            BuildMI(*MBB, MII, MI->getDebugLoc(),
                    TII->get(Hexagon::TFRI_cPt_f), DestReg).
                    addReg(SrcReg1).
                    addFPImm(MI->getOperand(2).getFPImm());
            BuildMI(*MBB, MII, MI->getDebugLoc(),
                    TII->get(Hexagon::TFRI_cNotPt_f), DestReg).
                    addReg(SrcReg1).
                    addFPImm(MI->getOperand(3).getFPImm());
          }
          MII = MBB->erase(MI);
          --MII;
          break;
        }
      }
    }
  }
  return true;
}

}

//===----------------------------------------------------------------------===//
//                         Public Constructor Functions
//===----------------------------------------------------------------------===//

FunctionPass *llvm::createHexagonSplitTFRCondSets(HexagonTargetMachine &TM) {
  return new HexagonSplitTFRCondSets(TM);
}