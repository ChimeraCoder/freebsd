
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

#include "NVPTXFrameLowering.h"
#include "NVPTX.h"
#include "NVPTXRegisterInfo.h"
#include "NVPTXSubtarget.h"
#include "NVPTXTargetMachine.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/Target/TargetInstrInfo.h"

using namespace llvm;

bool NVPTXFrameLowering::hasFP(const MachineFunction &MF) const { return true; }

void NVPTXFrameLowering::emitPrologue(MachineFunction &MF) const {
  if (MF.getFrameInfo()->hasStackObjects()) {
    MachineBasicBlock &MBB = MF.front();
    // Insert "mov.u32 %SP, %Depot"
    MachineBasicBlock::iterator MBBI = MBB.begin();
    // This instruction really occurs before first instruction
    // in the BB, so giving it no debug location.
    DebugLoc dl = DebugLoc();

    if (tm.getSubtargetImpl()->hasGenericLdSt()) {
      // mov %SPL, %depot;
      // cvta.local %SP, %SPL;
      if (is64bit) {
        MachineInstr *MI = BuildMI(
            MBB, MBBI, dl, tm.getInstrInfo()->get(NVPTX::cvta_local_yes_64),
            NVPTX::VRFrame).addReg(NVPTX::VRFrameLocal);
        BuildMI(MBB, MI, dl, tm.getInstrInfo()->get(NVPTX::IMOV64rr),
                NVPTX::VRFrameLocal).addReg(NVPTX::VRDepot);
      } else {
        MachineInstr *MI = BuildMI(
            MBB, MBBI, dl, tm.getInstrInfo()->get(NVPTX::cvta_local_yes),
            NVPTX::VRFrame).addReg(NVPTX::VRFrameLocal);
        BuildMI(MBB, MI, dl, tm.getInstrInfo()->get(NVPTX::IMOV32rr),
                NVPTX::VRFrameLocal).addReg(NVPTX::VRDepot);
      }
    } else {
      // mov %SP, %depot;
      if (is64bit)
        BuildMI(MBB, MBBI, dl, tm.getInstrInfo()->get(NVPTX::IMOV64rr),
                NVPTX::VRFrame).addReg(NVPTX::VRDepot);
      else
        BuildMI(MBB, MBBI, dl, tm.getInstrInfo()->get(NVPTX::IMOV32rr),
                NVPTX::VRFrame).addReg(NVPTX::VRDepot);
    }
  }
}

void NVPTXFrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {}

// This function eliminates ADJCALLSTACKDOWN,
// ADJCALLSTACKUP pseudo instructions
void NVPTXFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  // Simply discard ADJCALLSTACKDOWN,
  // ADJCALLSTACKUP instructions.
  MBB.erase(I);
}