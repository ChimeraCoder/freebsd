
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

#include "MipsMachineFunction.h"
#include "MCTargetDesc/MipsBaseInfo.h"
#include "MipsInstrInfo.h"
#include "MipsSubtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

static cl::opt<bool>
FixGlobalBaseReg("mips-fix-global-base-reg", cl::Hidden, cl::init(true),
                 cl::desc("Always use $gp as the global base register."));

bool MipsFunctionInfo::globalBaseRegSet() const {
  return GlobalBaseReg;
}

unsigned MipsFunctionInfo::getGlobalBaseReg() {
  // Return if it has already been initialized.
  if (GlobalBaseReg)
    return GlobalBaseReg;

  const MipsSubtarget &ST = MF.getTarget().getSubtarget<MipsSubtarget>();

  const TargetRegisterClass *RC;
  if (ST.inMips16Mode())
    RC=(const TargetRegisterClass*)&Mips::CPU16RegsRegClass;
  else
    RC = ST.isABI_N64() ?
      (const TargetRegisterClass*)&Mips::CPU64RegsRegClass :
      (const TargetRegisterClass*)&Mips::CPURegsRegClass;
  return GlobalBaseReg = MF.getRegInfo().createVirtualRegister(RC);
}

bool MipsFunctionInfo::mips16SPAliasRegSet() const {
  return Mips16SPAliasReg;
}
unsigned MipsFunctionInfo::getMips16SPAliasReg() {
  // Return if it has already been initialized.
  if (Mips16SPAliasReg)
    return Mips16SPAliasReg;

  const TargetRegisterClass *RC;
  RC=(const TargetRegisterClass*)&Mips::CPU16RegsRegClass;
  return Mips16SPAliasReg = MF.getRegInfo().createVirtualRegister(RC);
}

void MipsFunctionInfo::createEhDataRegsFI() {
  for (int I = 0; I < 4; ++I) {
    const MipsSubtarget &ST = MF.getTarget().getSubtarget<MipsSubtarget>();
    const TargetRegisterClass *RC = ST.isABI_N64() ?
        &Mips::CPU64RegsRegClass : &Mips::CPURegsRegClass;

    EhDataRegFI[I] = MF.getFrameInfo()->CreateStackObject(RC->getSize(),
        RC->getAlignment(), false);
  }
}

bool MipsFunctionInfo::isEhDataRegFI(int FI) const {
  return CallsEhReturn && (FI == EhDataRegFI[0] || FI == EhDataRegFI[1]
                        || FI == EhDataRegFI[2] || FI == EhDataRegFI[3]);
}

void MipsFunctionInfo::anchor() { }