
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

#define DEBUG_TYPE "processimplicitdefs"

#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"

using namespace llvm;

namespace {
/// Process IMPLICIT_DEF instructions and make sure there is one implicit_def
/// for each use. Add isUndef marker to implicit_def defs and their uses.
class ProcessImplicitDefs : public MachineFunctionPass {
  const TargetInstrInfo *TII;
  const TargetRegisterInfo *TRI;
  MachineRegisterInfo *MRI;

  SmallSetVector<MachineInstr*, 16> WorkList;

  void processImplicitDef(MachineInstr *MI);
  bool canTurnIntoImplicitDef(MachineInstr *MI);

public:
  static char ID;

  ProcessImplicitDefs() : MachineFunctionPass(ID) {
    initializeProcessImplicitDefsPass(*PassRegistry::getPassRegistry());
  }

  virtual void getAnalysisUsage(AnalysisUsage &au) const;

  virtual bool runOnMachineFunction(MachineFunction &fn);
};
} // end anonymous namespace

char ProcessImplicitDefs::ID = 0;
char &llvm::ProcessImplicitDefsID = ProcessImplicitDefs::ID;

INITIALIZE_PASS_BEGIN(ProcessImplicitDefs, "processimpdefs",
                "Process Implicit Definitions", false, false)
INITIALIZE_PASS_END(ProcessImplicitDefs, "processimpdefs",
                "Process Implicit Definitions", false, false)

void ProcessImplicitDefs::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addPreserved<AliasAnalysis>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool ProcessImplicitDefs::canTurnIntoImplicitDef(MachineInstr *MI) {
  if (!MI->isCopyLike() &&
      !MI->isInsertSubreg() &&
      !MI->isRegSequence() &&
      !MI->isPHI())
    return false;
  for (MIOperands MO(MI); MO.isValid(); ++MO)
    if (MO->isReg() && MO->isUse() && MO->readsReg())
      return false;
  return true;
}

void ProcessImplicitDefs::processImplicitDef(MachineInstr *MI) {
  DEBUG(dbgs() << "Processing " << *MI);
  unsigned Reg = MI->getOperand(0).getReg();

  if (TargetRegisterInfo::isVirtualRegister(Reg)) {
    // For virtual regiusters, mark all uses as <undef>, and convert users to
    // implicit-def when possible.
    for (MachineRegisterInfo::use_nodbg_iterator UI =
         MRI->use_nodbg_begin(Reg),
         UE = MRI->use_nodbg_end(); UI != UE; ++UI) {
      MachineOperand &MO = UI.getOperand();
      MO.setIsUndef();
      MachineInstr *UserMI = MO.getParent();
      if (!canTurnIntoImplicitDef(UserMI))
        continue;
      DEBUG(dbgs() << "Converting to IMPLICIT_DEF: " << *UserMI);
      UserMI->setDesc(TII->get(TargetOpcode::IMPLICIT_DEF));
      WorkList.insert(UserMI);
    }
    MI->eraseFromParent();
    return;
  }

  // This is a physreg implicit-def.
  // Look for the first instruction to use or define an alias.
  MachineBasicBlock::instr_iterator UserMI = MI;
  MachineBasicBlock::instr_iterator UserE = MI->getParent()->instr_end();
  bool Found = false;
  for (++UserMI; UserMI != UserE; ++UserMI) {
    for (MIOperands MO(UserMI); MO.isValid(); ++MO) {
      if (!MO->isReg())
        continue;
      unsigned UserReg = MO->getReg();
      if (!TargetRegisterInfo::isPhysicalRegister(UserReg) ||
          !TRI->regsOverlap(Reg, UserReg))
        continue;
      // UserMI uses or redefines Reg. Set <undef> flags on all uses.
      Found = true;
      if (MO->isUse())
        MO->setIsUndef();
    }
    if (Found)
      break;
  }

  // If we found the using MI, we can erase the IMPLICIT_DEF.
  if (Found) {
    DEBUG(dbgs() << "Physreg user: " << *UserMI);
    MI->eraseFromParent();
    return;
  }

  // Using instr wasn't found, it could be in another block.
  // Leave the physreg IMPLICIT_DEF, but trim any extra operands.
  for (unsigned i = MI->getNumOperands() - 1; i; --i)
    MI->RemoveOperand(i);
  DEBUG(dbgs() << "Keeping physreg: " << *MI);
}

/// processImplicitDefs - Process IMPLICIT_DEF instructions and turn them into
/// <undef> operands.
bool ProcessImplicitDefs::runOnMachineFunction(MachineFunction &MF) {

  DEBUG(dbgs() << "********** PROCESS IMPLICIT DEFS **********\n"
               << "********** Function: " << MF.getName() << '\n');

  bool Changed = false;

  TII = MF.getTarget().getInstrInfo();
  TRI = MF.getTarget().getRegisterInfo();
  MRI = &MF.getRegInfo();
  assert(MRI->isSSA() && "ProcessImplicitDefs only works on SSA form.");
  assert(WorkList.empty() && "Inconsistent worklist state");

  for (MachineFunction::iterator MFI = MF.begin(), MFE = MF.end();
       MFI != MFE; ++MFI) {
    // Scan the basic block for implicit defs.
    for (MachineBasicBlock::instr_iterator MBBI = MFI->instr_begin(),
         MBBE = MFI->instr_end(); MBBI != MBBE; ++MBBI)
      if (MBBI->isImplicitDef())
        WorkList.insert(MBBI);

    if (WorkList.empty())
      continue;

    DEBUG(dbgs() << "BB#" << MFI->getNumber() << " has " << WorkList.size()
                 << " implicit defs.\n");
    Changed = true;

    // Drain the WorkList to recursively process any new implicit defs.
    do processImplicitDef(WorkList.pop_back_val());
    while (!WorkList.empty());
  }
  return Changed;
}