
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

#include "llvm/Analysis/Passes.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
  class ModuleDebugInfoPrinter : public ModulePass {
    DebugInfoFinder Finder;
  public:
    static char ID; // Pass identification, replacement for typeid
    ModuleDebugInfoPrinter() : ModulePass(ID) {
      initializeModuleDebugInfoPrinterPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnModule(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
    virtual void print(raw_ostream &O, const Module *M) const;
  };
}

char ModuleDebugInfoPrinter::ID = 0;
INITIALIZE_PASS(ModuleDebugInfoPrinter, "module-debuginfo",
                "Decodes module-level debug info", false, true)

ModulePass *llvm::createModuleDebugInfoPrinterPass() {
  return new ModuleDebugInfoPrinter();
}

bool ModuleDebugInfoPrinter::runOnModule(Module &M) {
  Finder.processModule(M);
  return false;
}

void ModuleDebugInfoPrinter::print(raw_ostream &O, const Module *M) const {
  for (DebugInfoFinder::iterator I = Finder.compile_unit_begin(),
       E = Finder.compile_unit_end(); I != E; ++I) {
    O << "Compile Unit: ";
    DICompileUnit(*I).print(O);
    O << '\n';
  }

  for (DebugInfoFinder::iterator I = Finder.subprogram_begin(),
       E = Finder.subprogram_end(); I != E; ++I) {
    O << "Subprogram: ";
    DISubprogram(*I).print(O);
    O << '\n';
  }

  for (DebugInfoFinder::iterator I = Finder.global_variable_begin(),
       E = Finder.global_variable_end(); I != E; ++I) {
    O << "GlobalVariable: ";
    DIGlobalVariable(*I).print(O);
    O << '\n';
  }

  for (DebugInfoFinder::iterator I = Finder.type_begin(),
       E = Finder.type_end(); I != E; ++I) {
    O << "Type: ";
    DIType(*I).print(O);
    O << '\n';
  }
}