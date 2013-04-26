
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

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

TargetMachine *EngineBuilder::selectTarget() {
  Triple TT;

  // MCJIT can generate code for remote targets, but the old JIT and Interpreter
  // must use the host architecture.
  if (UseMCJIT && WhichEngine != EngineKind::Interpreter && M)
    TT.setTriple(M->getTargetTriple());

  return selectTarget(TT, MArch, MCPU, MAttrs);
}

/// selectTarget - Pick a target either via -march or by guessing the native
/// arch.  Add any CPU features specified via -mcpu or -mattr.
TargetMachine *EngineBuilder::selectTarget(const Triple &TargetTriple,
                              StringRef MArch,
                              StringRef MCPU,
                              const SmallVectorImpl<std::string>& MAttrs) {
  Triple TheTriple(TargetTriple);
  if (TheTriple.getTriple().empty())
    TheTriple.setTriple(sys::getProcessTriple());

  // Adjust the triple to match what the user requested.
  const Target *TheTarget = 0;
  if (!MArch.empty()) {
    for (TargetRegistry::iterator it = TargetRegistry::begin(),
           ie = TargetRegistry::end(); it != ie; ++it) {
      if (MArch == it->getName()) {
        TheTarget = &*it;
        break;
      }
    }

    if (!TheTarget) {
      if (ErrorStr)
        *ErrorStr = "No available targets are compatible with this -march, "
                    "see -version for the available targets.\n";
      return 0;
    }

    // Adjust the triple to match (if known), otherwise stick with the
    // requested/host triple.
    Triple::ArchType Type = Triple::getArchTypeForLLVMName(MArch);
    if (Type != Triple::UnknownArch)
      TheTriple.setArch(Type);
  } else {
    std::string Error;
    TheTarget = TargetRegistry::lookupTarget(TheTriple.getTriple(), Error);
    if (TheTarget == 0) {
      if (ErrorStr)
        *ErrorStr = Error;
      return 0;
    }
  }

  // Package up features to be passed to target/subtarget
  std::string FeaturesStr;
  if (!MAttrs.empty()) {
    SubtargetFeatures Features;
    for (unsigned i = 0; i != MAttrs.size(); ++i)
      Features.AddFeature(MAttrs[i]);
    FeaturesStr = Features.getString();
  }

  // Allocate a target...
  TargetMachine *Target = TheTarget->createTargetMachine(TheTriple.getTriple(),
                                                         MCPU, FeaturesStr,
                                                         Options,
                                                         RelocModel, CMModel,
                                                         OptLevel);
  assert(Target && "Could not allocate target machine!");
  return Target;
}