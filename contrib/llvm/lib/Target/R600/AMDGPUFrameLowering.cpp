
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
#include "AMDGPUFrameLowering.h"
#include "AMDGPURegisterInfo.h"
#include "R600MachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
AMDGPUFrameLowering::AMDGPUFrameLowering(StackDirection D, unsigned StackAl,
    int LAO, unsigned TransAl)
  : TargetFrameLowering(D, StackAl, LAO, TransAl) { }

AMDGPUFrameLowering::~AMDGPUFrameLowering() { }

unsigned AMDGPUFrameLowering::getStackWidth(const MachineFunction &MF) const {

  // XXX: Hardcoding to 1 for now.
  //
  // I think the StackWidth should stored as metadata associated with the
  // MachineFunction.  This metadata can either be added by a frontend, or
  // calculated by a R600 specific LLVM IR pass.
  //
  // The StackWidth determines how stack objects are laid out in memory.
  // For a vector stack variable, like: int4 stack[2], the data will be stored
  // in the following ways depending on the StackWidth.
  //
  // StackWidth = 1:
  //
  // T0.X = stack[0].x
  // T1.X = stack[0].y
  // T2.X = stack[0].z
  // T3.X = stack[0].w
  // T4.X = stack[1].x
  // T5.X = stack[1].y
  // T6.X = stack[1].z
  // T7.X = stack[1].w
  //
  // StackWidth = 2:
  //
  // T0.X = stack[0].x
  // T0.Y = stack[0].y
  // T1.X = stack[0].z
  // T1.Y = stack[0].w
  // T2.X = stack[1].x
  // T2.Y = stack[1].y
  // T3.X = stack[1].z
  // T3.Y = stack[1].w
  // 
  // StackWidth = 4:
  // T0.X = stack[0].x
  // T0.Y = stack[0].y
  // T0.Z = stack[0].z
  // T0.W = stack[0].w
  // T1.X = stack[1].x
  // T1.Y = stack[1].y
  // T1.Z = stack[1].z
  // T1.W = stack[1].w
  return 1;
}

/// \returns The number of registers allocated for \p FI.
int AMDGPUFrameLowering::getFrameIndexOffset(const MachineFunction &MF,
                                         int FI) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  unsigned Offset = 0;
  int UpperBound = FI == -1 ? MFI->getNumObjects() : FI;

  for (int i = MFI->getObjectIndexBegin(); i < UpperBound; ++i) {
    const AllocaInst *Alloca = MFI->getObjectAllocation(i);
    unsigned ArrayElements;
    const Type *AllocaType = Alloca->getAllocatedType();
    const Type *ElementType;

    if (AllocaType->isArrayTy()) {
      ArrayElements = AllocaType->getArrayNumElements();
      ElementType = AllocaType->getArrayElementType();
    } else {
      ArrayElements = 1;
      ElementType = AllocaType;
    }

    unsigned VectorElements;
    if (ElementType->isVectorTy()) {
      VectorElements = ElementType->getVectorNumElements();
    } else {
      VectorElements = 1;
    }

    Offset += (VectorElements / getStackWidth(MF)) * ArrayElements;
  }
  return Offset;
}

const TargetFrameLowering::SpillSlot *
AMDGPUFrameLowering::getCalleeSavedSpillSlots(unsigned &NumEntries) const {
  NumEntries = 0;
  return 0;
}
void
AMDGPUFrameLowering::emitPrologue(MachineFunction &MF) const {
}
void
AMDGPUFrameLowering::emitEpilogue(MachineFunction &MF,
                                  MachineBasicBlock &MBB) const {
}

bool
AMDGPUFrameLowering::hasFP(const MachineFunction &MF) const {
  return false;
}