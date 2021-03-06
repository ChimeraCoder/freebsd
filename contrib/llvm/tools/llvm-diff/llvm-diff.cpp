
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

#include "DiffLog.h"
#include "DifferenceEngine.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <utility>


using namespace llvm;

/// Reads a module from a file.  On error, messages are written to stderr
/// and null is returned.
static Module *ReadModule(LLVMContext &Context, StringRef Name) {
  SMDiagnostic Diag;
  Module *M = ParseIRFile(Name, Diag, Context);
  if (!M)
    Diag.print("llvm-diff", errs());
  return M;
}

static void diffGlobal(DifferenceEngine &Engine, Module *L, Module *R,
                       StringRef Name) {
  // Drop leading sigils from the global name.
  if (Name.startswith("@")) Name = Name.substr(1);

  Function *LFn = L->getFunction(Name);
  Function *RFn = R->getFunction(Name);
  if (LFn && RFn)
    Engine.diff(LFn, RFn);
  else if (!LFn && !RFn)
    errs() << "No function named @" << Name << " in either module\n";
  else if (!LFn)
    errs() << "No function named @" << Name << " in left module\n";
  else
    errs() << "No function named @" << Name << " in right module\n";
}

static cl::opt<std::string> LeftFilename(cl::Positional,
                                         cl::desc("<first file>"),
                                         cl::Required);
static cl::opt<std::string> RightFilename(cl::Positional,
                                          cl::desc("<second file>"),
                                          cl::Required);
static cl::list<std::string> GlobalsToCompare(cl::Positional,
                                              cl::desc("<globals to compare>"));

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv);

  LLVMContext Context;
  
  // Load both modules.  Die if that fails.
  Module *LModule = ReadModule(Context, LeftFilename);
  Module *RModule = ReadModule(Context, RightFilename);
  if (!LModule || !RModule) return 1;

  DiffConsumer Consumer;
  DifferenceEngine Engine(Consumer);

  // If any global names were given, just diff those.
  if (!GlobalsToCompare.empty()) {
    for (unsigned I = 0, E = GlobalsToCompare.size(); I != E; ++I)
      diffGlobal(Engine, LModule, RModule, GlobalsToCompare[I]);

  // Otherwise, diff everything in the module.
  } else {
    Engine.diff(LModule, RModule);
  }

  delete LModule;
  delete RModule;

  return Consumer.hadDifferences();
}