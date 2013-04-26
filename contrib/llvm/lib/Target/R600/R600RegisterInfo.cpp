
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

#include "R600RegisterInfo.h"
#include "AMDGPUTargetMachine.h"
#include "R600Defines.h"
#include "R600InstrInfo.h"
#include "R600MachineFunctionInfo.h"

using namespace llvm;

R600RegisterInfo::R600RegisterInfo(AMDGPUTargetMachine &tm,
    const TargetInstrInfo &tii)
: AMDGPURegisterInfo(tm, tii),
  TM(tm),
  TII(tii)
  { }

BitVector R600RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(AMDGPU::ZERO);
  Reserved.set(AMDGPU::HALF);
  Reserved.set(AMDGPU::ONE);
  Reserved.set(AMDGPU::ONE_INT);
  Reserved.set(AMDGPU::NEG_HALF);
  Reserved.set(AMDGPU::NEG_ONE);
  Reserved.set(AMDGPU::PV_X);
  Reserved.set(AMDGPU::ALU_LITERAL_X);
  Reserved.set(AMDGPU::ALU_CONST);
  Reserved.set(AMDGPU::PREDICATE_BIT);
  Reserved.set(AMDGPU::PRED_SEL_OFF);
  Reserved.set(AMDGPU::PRED_SEL_ZERO);
  Reserved.set(AMDGPU::PRED_SEL_ONE);

  for (TargetRegisterClass::iterator I = AMDGPU::R600_AddrRegClass.begin(),
                        E = AMDGPU::R600_AddrRegClass.end(); I != E; ++I) {
    Reserved.set(*I);
  }

  for (TargetRegisterClass::iterator I = AMDGPU::TRegMemRegClass.begin(),
                                     E = AMDGPU::TRegMemRegClass.end();
                                     I !=  E; ++I) {
    Reserved.set(*I);
  }

  const R600InstrInfo *RII = static_cast<const R600InstrInfo*>(&TII);
  std::vector<unsigned> IndirectRegs = RII->getIndirectReservedRegs(MF);
  for (std::vector<unsigned>::iterator I = IndirectRegs.begin(),
                                       E = IndirectRegs.end();
                                       I != E; ++I) {
    Reserved.set(*I);
  }
  return Reserved;
}

const TargetRegisterClass *
R600RegisterInfo::getISARegClass(const TargetRegisterClass * rc) const {
  switch (rc->getID()) {
  case AMDGPU::GPRF32RegClassID:
  case AMDGPU::GPRI32RegClassID:
    return &AMDGPU::R600_Reg32RegClass;
  default: return rc;
  }
}

unsigned R600RegisterInfo::getHWRegChan(unsigned reg) const {
  return this->getEncodingValue(reg) >> HW_CHAN_SHIFT;
}

const TargetRegisterClass * R600RegisterInfo::getCFGStructurizerRegClass(
                                                                   MVT VT) const {
  switch(VT.SimpleTy) {
  default:
  case MVT::i32: return &AMDGPU::R600_TReg32RegClass;
  }
}

unsigned R600RegisterInfo::getSubRegFromChannel(unsigned Channel) const {
  switch (Channel) {
    default: assert(!"Invalid channel index"); return 0;
    case 0: return AMDGPU::sub0;
    case 1: return AMDGPU::sub1;
    case 2: return AMDGPU::sub2;
    case 3: return AMDGPU::sub3;
  }
}