
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

#include "XCoreTargetMachine.h"
#include "XCore.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

/// XCoreTargetMachine ctor - Create an ILP32 architecture model
///
XCoreTargetMachine::XCoreTargetMachine(const Target &T, StringRef TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Reloc::Model RM, CodeModel::Model CM,
                                       CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS),
    DL("e-p:32:32:32-a0:0:32-f32:32:32-f64:32:32-i1:8:32-i8:8:32-"
               "i16:16:32-i32:32:32-i64:32:32-n32"),
    InstrInfo(),
    FrameLowering(Subtarget),
    TLInfo(*this),
    TSInfo(*this) {
}

namespace {
/// XCore Code Generator Pass Configuration Options.
class XCorePassConfig : public TargetPassConfig {
public:
  XCorePassConfig(XCoreTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  XCoreTargetMachine &getXCoreTargetMachine() const {
    return getTM<XCoreTargetMachine>();
  }

  virtual bool addInstSelector();
};
} // namespace

TargetPassConfig *XCoreTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new XCorePassConfig(this, PM);
}

bool XCorePassConfig::addInstSelector() {
  addPass(createXCoreISelDag(getXCoreTargetMachine(), getOptLevel()));
  return false;
}

// Force static initialization.
extern "C" void LLVMInitializeXCoreTarget() {
  RegisterTargetMachine<XCoreTargetMachine> X(TheXCoreTarget);
}