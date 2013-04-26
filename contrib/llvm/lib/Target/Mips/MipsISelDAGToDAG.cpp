
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

#define DEBUG_TYPE "mips-isel"
#include "MipsISelDAGToDAG.h"
#include "Mips16ISelDAGToDAG.h"
#include "MipsSEISelDAGToDAG.h"
#include "Mips.h"
#include "MCTargetDesc/MipsBaseInfo.h"
#include "MipsAnalyzeImmediate.h"
#include "MipsMachineFunction.h"
#include "MipsRegisterInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// MipsDAGToDAGISel - MIPS specific code to select MIPS machine
// instructions for SelectionDAG operations.
//===----------------------------------------------------------------------===//

bool MipsDAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
  bool Ret = SelectionDAGISel::runOnMachineFunction(MF);

  processFunctionAfterISel(MF);

  return Ret;
}

/// getGlobalBaseReg - Output the instructions required to put the
/// GOT address into a register.
SDNode *MipsDAGToDAGISel::getGlobalBaseReg() {
  unsigned GlobalBaseReg = MF->getInfo<MipsFunctionInfo>()->getGlobalBaseReg();
  return CurDAG->getRegister(GlobalBaseReg, TLI.getPointerTy()).getNode();
}

/// ComplexPattern used on MipsInstrInfo
/// Used on Mips Load/Store instructions
bool MipsDAGToDAGISel::selectAddrRegImm(SDValue Addr, SDValue &Base,
                                        SDValue &Offset) const {
  llvm_unreachable("Unimplemented function.");
  return false;
}

bool MipsDAGToDAGISel::selectAddrDefault(SDValue Addr, SDValue &Base,
                                         SDValue &Offset) const {
  llvm_unreachable("Unimplemented function.");
  return false;
}

bool MipsDAGToDAGISel::selectIntAddr(SDValue Addr, SDValue &Base,
                                     SDValue &Offset) const {
  llvm_unreachable("Unimplemented function.");
  return false;
}

bool MipsDAGToDAGISel::selectAddr16(SDNode *Parent, SDValue N, SDValue &Base,
                                    SDValue &Offset, SDValue &Alias) {
  llvm_unreachable("Unimplemented function.");
  return false;
}

/// Select instructions not customized! Used for
/// expanded, promoted and normal instructions
SDNode* MipsDAGToDAGISel::Select(SDNode *Node) {
  unsigned Opcode = Node->getOpcode();

  // Dump information about the Node being selected
  DEBUG(errs() << "Selecting: "; Node->dump(CurDAG); errs() << "\n");

  // If we have a custom node, we already have selected!
  if (Node->isMachineOpcode()) {
    DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
    return NULL;
  }

  // See if subclasses can handle this node.
  std::pair<bool, SDNode*> Ret = selectNode(Node);

  if (Ret.first)
    return Ret.second;

  switch(Opcode) {
  default: break;

  // Get target GOT address.
  case ISD::GLOBAL_OFFSET_TABLE:
    return getGlobalBaseReg();

#ifndef NDEBUG
  case ISD::LOAD:
  case ISD::STORE:
    assert(cast<MemSDNode>(Node)->getMemoryVT().getSizeInBits() / 8 <=
           cast<MemSDNode>(Node)->getAlignment() &&
           "Unexpected unaligned loads/stores.");
    break;
#endif
  }

  // Select the default instruction
  SDNode *ResNode = SelectCode(Node);

  DEBUG(errs() << "=> ");
  if (ResNode == NULL || ResNode == Node)
    DEBUG(Node->dump(CurDAG));
  else
    DEBUG(ResNode->dump(CurDAG));
  DEBUG(errs() << "\n");
  return ResNode;
}

bool MipsDAGToDAGISel::
SelectInlineAsmMemoryOperand(const SDValue &Op, char ConstraintCode,
                             std::vector<SDValue> &OutOps) {
  assert(ConstraintCode == 'm' && "unexpected asm memory constraint");
  OutOps.push_back(Op);
  return false;
}

/// createMipsISelDag - This pass converts a legalized DAG into a
/// MIPS-specific DAG, ready for instruction scheduling.
FunctionPass *llvm::createMipsISelDag(MipsTargetMachine &TM) {
  if (TM.getSubtargetImpl()->inMips16Mode())
    return llvm::createMips16ISelDag(TM);

  return llvm::createMipsSEISelDag(TM);
}