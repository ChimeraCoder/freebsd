
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

#include "Hexagon.h"
#include "HexagonMachineFunctionInfo.h"
#include "HexagonSubtarget.h"
#include "HexagonTargetMachine.h"
#include "llvm/ADT/Statistic.h"
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

class HexagonExpandPredSpillCode : public MachineFunctionPass {
    HexagonTargetMachine& QTM;
    const HexagonSubtarget &QST;

 public:
    static char ID;
    HexagonExpandPredSpillCode(HexagonTargetMachine& TM) :
      MachineFunctionPass(ID), QTM(TM), QST(*TM.getSubtargetImpl()) {}

    const char *getPassName() const {
      return "Hexagon Expand Predicate Spill Code";
    }
    bool runOnMachineFunction(MachineFunction &Fn);
};


char HexagonExpandPredSpillCode::ID = 0;


bool HexagonExpandPredSpillCode::runOnMachineFunction(MachineFunction &Fn) {

  const HexagonInstrInfo *TII = QTM.getInstrInfo();

  // Loop over all of the basic blocks.
  for (MachineFunction::iterator MBBb = Fn.begin(), MBBe = Fn.end();
       MBBb != MBBe; ++MBBb) {
    MachineBasicBlock* MBB = MBBb;
    // Traverse the basic block.
    for (MachineBasicBlock::iterator MII = MBB->begin(); MII != MBB->end();
         ++MII) {
      MachineInstr *MI = MII;
      int Opc = MI->getOpcode();
      if (Opc == Hexagon::STriw_pred) {
        // STriw_pred [R30], ofst, SrcReg;
        unsigned FP = MI->getOperand(0).getReg();
        assert(FP == QTM.getRegisterInfo()->getFrameRegister() &&
               "Not a Frame Pointer, Nor a Spill Slot");
        assert(MI->getOperand(1).isImm() && "Not an offset");
        int Offset = MI->getOperand(1).getImm();
        int SrcReg = MI->getOperand(2).getReg();
        assert(Hexagon::PredRegsRegClass.contains(SrcReg) &&
               "Not a predicate register");
        if (!TII->isValidOffset(Hexagon::STriw_indexed, Offset)) {
          if (!TII->isValidOffset(Hexagon::ADD_ri, Offset)) {
            BuildMI(*MBB, MII, MI->getDebugLoc(),
                    TII->get(Hexagon::CONST32_Int_Real),
                      HEXAGON_RESERVED_REG_1).addImm(Offset);
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::ADD_rr),
                    HEXAGON_RESERVED_REG_1)
              .addReg(FP).addReg(HEXAGON_RESERVED_REG_1);
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::TFR_RsPd),
                      HEXAGON_RESERVED_REG_2).addReg(SrcReg);
            BuildMI(*MBB, MII, MI->getDebugLoc(),
                    TII->get(Hexagon::STriw_indexed))
              .addReg(HEXAGON_RESERVED_REG_1)
              .addImm(0).addReg(HEXAGON_RESERVED_REG_2);
          } else {
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::ADD_ri),
                      HEXAGON_RESERVED_REG_1).addReg(FP).addImm(Offset);
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::TFR_RsPd),
                      HEXAGON_RESERVED_REG_2).addReg(SrcReg);
            BuildMI(*MBB, MII, MI->getDebugLoc(),
                          TII->get(Hexagon::STriw_indexed))
              .addReg(HEXAGON_RESERVED_REG_1)
              .addImm(0)
              .addReg(HEXAGON_RESERVED_REG_2);
          }
        } else {
          BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::TFR_RsPd),
                    HEXAGON_RESERVED_REG_2).addReg(SrcReg);
          BuildMI(*MBB, MII, MI->getDebugLoc(),
                        TII->get(Hexagon::STriw_indexed)).
                    addReg(FP).addImm(Offset).addReg(HEXAGON_RESERVED_REG_2);
        }
        MII = MBB->erase(MI);
        --MII;
      } else if (Opc == Hexagon::LDriw_pred) {
        // DstReg = LDriw_pred [R30], ofst.
        int DstReg = MI->getOperand(0).getReg();
        assert(Hexagon::PredRegsRegClass.contains(DstReg) &&
               "Not a predicate register");
        unsigned FP = MI->getOperand(1).getReg();
        assert(FP == QTM.getRegisterInfo()->getFrameRegister() &&
               "Not a Frame Pointer, Nor a Spill Slot");
        assert(MI->getOperand(2).isImm() && "Not an offset");
        int Offset = MI->getOperand(2).getImm();
        if (!TII->isValidOffset(Hexagon::LDriw, Offset)) {
          if (!TII->isValidOffset(Hexagon::ADD_ri, Offset)) {
            BuildMI(*MBB, MII, MI->getDebugLoc(),
                    TII->get(Hexagon::CONST32_Int_Real),
                      HEXAGON_RESERVED_REG_1).addImm(Offset);
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::ADD_rr),
                    HEXAGON_RESERVED_REG_1)
              .addReg(FP)
              .addReg(HEXAGON_RESERVED_REG_1);
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::LDriw),
                      HEXAGON_RESERVED_REG_2)
              .addReg(HEXAGON_RESERVED_REG_1)
              .addImm(0);
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::TFR_PdRs),
                      DstReg).addReg(HEXAGON_RESERVED_REG_2);
          } else {
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::ADD_ri),
                      HEXAGON_RESERVED_REG_1).addReg(FP).addImm(Offset);
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::LDriw),
                      HEXAGON_RESERVED_REG_2)
              .addReg(HEXAGON_RESERVED_REG_1)
              .addImm(0);
            BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::TFR_PdRs),
                      DstReg).addReg(HEXAGON_RESERVED_REG_2);
          }
        } else {
          BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::LDriw),
                    HEXAGON_RESERVED_REG_2).addReg(FP).addImm(Offset);
          BuildMI(*MBB, MII, MI->getDebugLoc(), TII->get(Hexagon::TFR_PdRs),
                    DstReg).addReg(HEXAGON_RESERVED_REG_2);
        }
        MII = MBB->erase(MI);
        --MII;
      }
    }
  }

  return true;
}

}

//===----------------------------------------------------------------------===//
//                         Public Constructor Functions
//===----------------------------------------------------------------------===//

FunctionPass *llvm::createHexagonExpandPredSpillCode(HexagonTargetMachine &TM) {
  return new HexagonExpandPredSpillCode(TM);
}