
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

#include "AMDGPUMCInstLower.h"
#include "AMDGPUAsmPrinter.h"
#include "R600InstrInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/Constants.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

AMDGPUMCInstLower::AMDGPUMCInstLower(MCContext &ctx):
  Ctx(ctx)
{ }

void AMDGPUMCInstLower::lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumExplicitOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);

    MCOperand MCOp;
    switch (MO.getType()) {
    default:
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_FPImmediate: {
      const APFloat &FloatValue = MO.getFPImm()->getValueAPF();
      assert(&FloatValue.getSemantics() == &APFloat::IEEEsingle &&
             "Only floating point immediates are supported at the moment.");
      MCOp = MCOperand::CreateFPImm(FloatValue.convertToFloat());
      break;
    }
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::CreateImm(MO.getImm());
      break;
    case MachineOperand::MO_Register:
      MCOp = MCOperand::CreateReg(MO.getReg());
      break;
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::CreateExpr(MCSymbolRefExpr::Create(
                                   MO.getMBB()->getSymbol(), Ctx));
    }
    OutMI.addOperand(MCOp);
  }
}

void AMDGPUAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  AMDGPUMCInstLower MCInstLowering(OutContext);

  if (MI->isBundle()) {
    const MachineBasicBlock *MBB = MI->getParent();
    MachineBasicBlock::const_instr_iterator I = MI;
    ++I;
    while (I != MBB->end() && I->isInsideBundle()) {
      MCInst MCBundleInst;
      const MachineInstr *BundledInst = I;
      MCInstLowering.lower(BundledInst, MCBundleInst);
      OutStreamer.EmitInstruction(MCBundleInst);
      ++I;
    }
  } else {
    MCInst TmpInst;
    MCInstLowering.lower(MI, TmpInst);
    OutStreamer.EmitInstruction(TmpInst);
  }
}