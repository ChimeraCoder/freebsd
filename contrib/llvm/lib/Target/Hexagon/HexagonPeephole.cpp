
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


//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "hexagon-peephole"
#include "Hexagon.h"
#include "HexagonTargetMachine.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Constants.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include <algorithm>

using namespace llvm;

static cl::opt<bool> DisableHexagonPeephole("disable-hexagon-peephole",
    cl::Hidden, cl::ZeroOrMore, cl::init(false),
    cl::desc("Disable Peephole Optimization"));

static cl::opt<int>
DbgPNPCount("pnp-count", cl::init(-1), cl::Hidden,
  cl::desc("Maximum number of P=NOT(P) to be optimized"));

static cl::opt<bool> DisablePNotP("disable-hexagon-pnotp",
    cl::Hidden, cl::ZeroOrMore, cl::init(false),
    cl::desc("Disable Optimization of PNotP"));

static cl::opt<bool> DisableOptSZExt("disable-hexagon-optszext",
    cl::Hidden, cl::ZeroOrMore, cl::init(false),
    cl::desc("Disable Optimization of Sign/Zero Extends"));

namespace {
  struct HexagonPeephole : public MachineFunctionPass {
    const HexagonInstrInfo    *QII;
    const HexagonRegisterInfo *QRI;
    const MachineRegisterInfo *MRI;

  public:
    static char ID;
    HexagonPeephole() : MachineFunctionPass(ID) { }

    bool runOnMachineFunction(MachineFunction &MF);

    const char *getPassName() const {
      return "Hexagon optimize redundant zero and size extends";
    }

    void getAnalysisUsage(AnalysisUsage &AU) const {
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    void ChangeOpInto(MachineOperand &Dst, MachineOperand &Src);
  };
}

char HexagonPeephole::ID = 0;

bool HexagonPeephole::runOnMachineFunction(MachineFunction &MF) {

  QII = static_cast<const HexagonInstrInfo *>(MF.getTarget().
                                        getInstrInfo());
  QRI = static_cast<const HexagonRegisterInfo *>(MF.getTarget().
                                       getRegisterInfo());
  MRI = &MF.getRegInfo();

  DenseMap<unsigned, unsigned> PeepholeMap;
  DenseMap<unsigned, std::pair<unsigned, unsigned> > PeepholeDoubleRegsMap;

  if (DisableHexagonPeephole) return false;

  // Loop over all of the basic blocks.
  for (MachineFunction::iterator MBBb = MF.begin(), MBBe = MF.end();
       MBBb != MBBe; ++MBBb) {
    MachineBasicBlock* MBB = MBBb;
    PeepholeMap.clear();
    PeepholeDoubleRegsMap.clear();

    // Traverse the basic block.
    for (MachineBasicBlock::iterator MII = MBB->begin(); MII != MBB->end();
                                     ++MII) {
      MachineInstr *MI = MII;
      // Look for sign extends:
      // %vreg170<def> = SXTW %vreg166
      if (!DisableOptSZExt && MI->getOpcode() == Hexagon::SXTW) {
        assert (MI->getNumOperands() == 2);
        MachineOperand &Dst = MI->getOperand(0);
        MachineOperand &Src  = MI->getOperand(1);
        unsigned DstReg = Dst.getReg();
        unsigned SrcReg = Src.getReg();
        // Just handle virtual registers.
        if (TargetRegisterInfo::isVirtualRegister(DstReg) &&
            TargetRegisterInfo::isVirtualRegister(SrcReg)) {
          // Map the following:
          // %vreg170<def> = SXTW %vreg166
          // PeepholeMap[170] = vreg166
          PeepholeMap[DstReg] = SrcReg;
        }
      }

      // Look for this sequence below
      // %vregDoubleReg1 = LSRd_ri %vregDoubleReg0, 32
      // %vregIntReg = COPY %vregDoubleReg1:subreg_loreg.
      // and convert into
      // %vregIntReg = COPY %vregDoubleReg0:subreg_hireg.
      if (MI->getOpcode() == Hexagon::LSRd_ri) {
        assert(MI->getNumOperands() == 3);
        MachineOperand &Dst = MI->getOperand(0);
        MachineOperand &Src1 = MI->getOperand(1);
        MachineOperand &Src2 = MI->getOperand(2);
        if (Src2.getImm() != 32)
          continue;
        unsigned DstReg = Dst.getReg();
        unsigned SrcReg = Src1.getReg();
        PeepholeDoubleRegsMap[DstReg] =
          std::make_pair(*&SrcReg, 1/*Hexagon::subreg_hireg*/);
      }

      // Look for P=NOT(P).
      if (!DisablePNotP &&
          (MI->getOpcode() == Hexagon::NOT_p)) {
        assert (MI->getNumOperands() == 2);
        MachineOperand &Dst = MI->getOperand(0);
        MachineOperand &Src  = MI->getOperand(1);
        unsigned DstReg = Dst.getReg();
        unsigned SrcReg = Src.getReg();
        // Just handle virtual registers.
        if (TargetRegisterInfo::isVirtualRegister(DstReg) &&
            TargetRegisterInfo::isVirtualRegister(SrcReg)) {
          // Map the following:
          // %vreg170<def> = NOT_xx %vreg166
          // PeepholeMap[170] = vreg166
          PeepholeMap[DstReg] = SrcReg;
        }
      }

      // Look for copy:
      // %vreg176<def> = COPY %vreg170:subreg_loreg
      if (!DisableOptSZExt && MI->isCopy()) {
        assert (MI->getNumOperands() == 2);
        MachineOperand &Dst = MI->getOperand(0);
        MachineOperand &Src  = MI->getOperand(1);

        // Make sure we are copying the lower 32 bits.
        if (Src.getSubReg() != Hexagon::subreg_loreg)
          continue;

        unsigned DstReg = Dst.getReg();
        unsigned SrcReg = Src.getReg();
        if (TargetRegisterInfo::isVirtualRegister(DstReg) &&
            TargetRegisterInfo::isVirtualRegister(SrcReg)) {
          // Try to find in the map.
          if (unsigned PeepholeSrc = PeepholeMap.lookup(SrcReg)) {
            // Change the 1st operand.
            MI->RemoveOperand(1);
            MI->addOperand(MachineOperand::CreateReg(PeepholeSrc, false));
          } else  {
            DenseMap<unsigned, std::pair<unsigned, unsigned> >::iterator DI =
              PeepholeDoubleRegsMap.find(SrcReg);
            if (DI != PeepholeDoubleRegsMap.end()) {
              std::pair<unsigned,unsigned> PeepholeSrc = DI->second;
              MI->RemoveOperand(1);
              MI->addOperand(MachineOperand::CreateReg(PeepholeSrc.first,
                                                       false /*isDef*/,
                                                       false /*isImp*/,
                                                       false /*isKill*/,
                                                       false /*isDead*/,
                                                       false /*isUndef*/,
                                                       false /*isEarlyClobber*/,
                                                       PeepholeSrc.second));
            }
          }
        }
      }

      // Look for Predicated instructions.
      if (!DisablePNotP) {
        bool Done = false;
        if (QII->isPredicated(MI)) {
          MachineOperand &Op0 = MI->getOperand(0);
          unsigned Reg0 = Op0.getReg();
          const TargetRegisterClass *RC0 = MRI->getRegClass(Reg0);
          if (RC0->getID() == Hexagon::PredRegsRegClassID) {
            // Handle instructions that have a prediate register in op0
            // (most cases of predicable instructions).
            if (TargetRegisterInfo::isVirtualRegister(Reg0)) {
              // Try to find in the map.
              if (unsigned PeepholeSrc = PeepholeMap.lookup(Reg0)) {
                // Change the 1st operand and, flip the opcode.
                MI->getOperand(0).setReg(PeepholeSrc);
                int NewOp = QII->getInvertedPredicatedOpcode(MI->getOpcode());
                MI->setDesc(QII->get(NewOp));
                Done = true;
              }
            }
          }
        }

        if (!Done) {
          // Handle special instructions.
          unsigned Op = MI->getOpcode();
          unsigned NewOp = 0;
          unsigned PR = 1, S1 = 2, S2 = 3;   // Operand indices.

          switch (Op) {
            case Hexagon::TFR_condset_rr:
            case Hexagon::TFR_condset_ii:
            case Hexagon::MUX_ii:
            case Hexagon::MUX_rr:
              NewOp = Op;
              break;
            case Hexagon::TFR_condset_ri:
              NewOp = Hexagon::TFR_condset_ir;
              break;
            case Hexagon::TFR_condset_ir:
              NewOp = Hexagon::TFR_condset_ri;
              break;
            case Hexagon::MUX_ri:
              NewOp = Hexagon::MUX_ir;
              break;
            case Hexagon::MUX_ir:
              NewOp = Hexagon::MUX_ri;
              break;
          }
          if (NewOp) {
            unsigned PSrc = MI->getOperand(PR).getReg();
            if (unsigned POrig = PeepholeMap.lookup(PSrc)) {
              MI->getOperand(PR).setReg(POrig);
              MI->setDesc(QII->get(NewOp));
              // Swap operands S1 and S2.
              MachineOperand Op1 = MI->getOperand(S1);
              MachineOperand Op2 = MI->getOperand(S2);
              ChangeOpInto(MI->getOperand(S1), Op2);
              ChangeOpInto(MI->getOperand(S2), Op1);
            }
          } // if (NewOp)
        } // if (!Done)

      } // if (!DisablePNotP)

    } // Instruction
  } // Basic Block
  return true;
}

void HexagonPeephole::ChangeOpInto(MachineOperand &Dst, MachineOperand &Src) {
  assert (&Dst != &Src && "Cannot duplicate into itself");
  switch (Dst.getType()) {
    case MachineOperand::MO_Register:
      if (Src.isReg()) {
        Dst.setReg(Src.getReg());
      } else if (Src.isImm()) {
        Dst.ChangeToImmediate(Src.getImm());
      } else {
        llvm_unreachable("Unexpected src operand type");
      }
      break;

    case MachineOperand::MO_Immediate:
      if (Src.isImm()) {
        Dst.setImm(Src.getImm());
      } else if (Src.isReg()) {
        Dst.ChangeToRegister(Src.getReg(), Src.isDef(), Src.isImplicit(),
                             Src.isKill(), Src.isDead(), Src.isUndef(),
                             Src.isDebug());
      } else {
        llvm_unreachable("Unexpected src operand type");
      }
      break;

    default:
      llvm_unreachable("Unexpected dst operand type");
      break;
  }
}

FunctionPass *llvm::createHexagonPeephole() {
  return new HexagonPeephole();
}