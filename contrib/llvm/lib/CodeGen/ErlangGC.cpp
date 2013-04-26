
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

#include "llvm/CodeGen/GCs.h"
#include "llvm/CodeGen/GCStrategy.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

namespace {

  class ErlangGC : public GCStrategy {
    MCSymbol *InsertLabel(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator MI,
                          DebugLoc DL) const;
  public:
    ErlangGC();
    bool findCustomSafePoints(GCFunctionInfo &FI, MachineFunction &MF);
  };

}

static GCRegistry::Add<ErlangGC>
X("erlang", "erlang-compatible garbage collector");

void llvm::linkErlangGC() { }

ErlangGC::ErlangGC() {
  InitRoots = false;
  NeededSafePoints = 1 << GC::PostCall;
  UsesMetadata = true;
  CustomRoots = false;
  CustomSafePoints = true;
}

MCSymbol *ErlangGC::InsertLabel(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                DebugLoc DL) const {
  const TargetInstrInfo* TII = MBB.getParent()->getTarget().getInstrInfo();
  MCSymbol *Label = MBB.getParent()->getContext().CreateTempSymbol();
  BuildMI(MBB, MI, DL, TII->get(TargetOpcode::GC_LABEL)).addSym(Label);
  return Label;
}

bool ErlangGC::findCustomSafePoints(GCFunctionInfo &FI, MachineFunction &MF) {
  for (MachineFunction::iterator BBI = MF.begin(), BBE = MF.end(); BBI != BBE;
       ++BBI)
    for (MachineBasicBlock::iterator MI = BBI->begin(), ME = BBI->end();
         MI != ME; ++MI)

      if (MI->getDesc().isCall()) {

        // Do not treat tail call sites as safe points.
        if (MI->getDesc().isTerminator())
          continue;

        /* Code copied from VisitCallPoint(...) */
        MachineBasicBlock::iterator RAI = MI; ++RAI;
        MCSymbol* Label = InsertLabel(*MI->getParent(), RAI, MI->getDebugLoc());
        FI.addSafePoint(GC::PostCall, Label, MI->getDebugLoc());
      }

  return false;
}