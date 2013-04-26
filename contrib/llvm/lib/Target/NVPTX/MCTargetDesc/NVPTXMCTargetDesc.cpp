
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

#include "NVPTXMCTargetDesc.h"
#include "NVPTXMCAsmInfo.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "NVPTXGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "NVPTXGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "NVPTXGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createNVPTXMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitNVPTXMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createNVPTXMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  // PTX does not have a return address register.
  InitNVPTXMCRegisterInfo(X, 0);
  return X;
}

static MCSubtargetInfo *
createNVPTXMCSubtargetInfo(StringRef TT, StringRef CPU, StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitNVPTXMCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCCodeGenInfo *createNVPTXMCCodeGenInfo(
    StringRef TT, Reloc::Model RM, CodeModel::Model CM, CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

// Force static initialization.
extern "C" void LLVMInitializeNVPTXTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<NVPTXMCAsmInfo> X(TheNVPTXTarget32);
  RegisterMCAsmInfo<NVPTXMCAsmInfo> Y(TheNVPTXTarget64);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheNVPTXTarget32,
                                        createNVPTXMCCodeGenInfo);
  TargetRegistry::RegisterMCCodeGenInfo(TheNVPTXTarget64,
                                        createNVPTXMCCodeGenInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheNVPTXTarget32, createNVPTXMCInstrInfo);
  TargetRegistry::RegisterMCInstrInfo(TheNVPTXTarget64, createNVPTXMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheNVPTXTarget32,
                                    createNVPTXMCRegisterInfo);
  TargetRegistry::RegisterMCRegInfo(TheNVPTXTarget64,
                                    createNVPTXMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheNVPTXTarget32,
                                          createNVPTXMCSubtargetInfo);
  TargetRegistry::RegisterMCSubtargetInfo(TheNVPTXTarget64,
                                          createNVPTXMCSubtargetInfo);

}