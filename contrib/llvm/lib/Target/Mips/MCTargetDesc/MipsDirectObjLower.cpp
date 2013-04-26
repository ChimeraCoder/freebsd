
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
#include "MipsInstrInfo.h"
#include "MCTargetDesc/MipsDirectObjLower.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"

using namespace llvm;

// If the D<shift> instruction has a shift amount that is greater
// than 31 (checked in calling routine), lower it to a D<shift>32 instruction
void Mips::LowerLargeShift(MCInst& Inst) {

  assert(Inst.getNumOperands() == 3 && "Invalid no. of operands for shift!");
  assert(Inst.getOperand(2).isImm());

  int64_t Shift = Inst.getOperand(2).getImm();
  if (Shift <= 31)
    return; // Do nothing
  Shift -= 32;

  // saminus32
  Inst.getOperand(2).setImm(Shift);

  switch (Inst.getOpcode()) {
  default:
    // Calling function is not synchronized
    llvm_unreachable("Unexpected shift instruction");
  case Mips::DSLL:
    Inst.setOpcode(Mips::DSLL32);
    return;
  case Mips::DSRL:
    Inst.setOpcode(Mips::DSRL32);
    return;
  case Mips::DSRA:
    Inst.setOpcode(Mips::DSRA32);
    return;
  }
}

// Pick a DEXT or DINS instruction variant based on the pos and size operands
void Mips::LowerDextDins(MCInst& InstIn) {
  int Opcode = InstIn.getOpcode();

  if (Opcode == Mips::DEXT)
    assert(InstIn.getNumOperands() == 4 &&
           "Invalid no. of machine operands for DEXT!");
  else // Only DEXT and DINS are possible
    assert(InstIn.getNumOperands() == 5 &&
           "Invalid no. of machine operands for DINS!");

  assert(InstIn.getOperand(2).isImm());
  int64_t pos = InstIn.getOperand(2).getImm();
  assert(InstIn.getOperand(3).isImm());
  int64_t size = InstIn.getOperand(3).getImm();

  if (size <= 32) {
    if (pos < 32)  // DEXT/DINS, do nothing
      return;
    // DEXTU/DINSU
    InstIn.getOperand(2).setImm(pos - 32);
    InstIn.setOpcode((Opcode == Mips::DEXT) ? Mips::DEXTU : Mips::DINSU);
    return;
  }
  // DEXTM/DINSM
  assert(pos < 32 && "DEXT/DINS cannot have both size and pos > 32");
  InstIn.getOperand(3).setImm(size - 32);
  InstIn.setOpcode((Opcode == Mips::DEXT) ? Mips::DEXTM : Mips::DINSM);
  return;
}