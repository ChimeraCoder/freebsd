
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

#include "llvm/IR/LLVMContext.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/Parser.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/SystemUtils.h"
#include "llvm/Support/ToolOutputFile.h"
#include <memory>
using namespace llvm;

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input .llvm file>"), cl::init("-"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Override output filename"),
               cl::value_desc("filename"));

static cl::opt<bool>
Force("f", cl::desc("Enable binary output on terminals"));

static cl::opt<bool>
DisableOutput("disable-output", cl::desc("Disable output"), cl::init(false));

static cl::opt<bool>
DumpAsm("d", cl::desc("Print assembly as parsed"), cl::Hidden);

static cl::opt<bool>
DisableVerify("disable-verify", cl::Hidden,
              cl::desc("Do not run verifier on input LLVM (dangerous!)"));

static void WriteOutputFile(const Module *M) {
  // Infer the output filename if needed.
  if (OutputFilename.empty()) {
    if (InputFilename == "-") {
      OutputFilename = "-";
    } else {
      std::string IFN = InputFilename;
      int Len = IFN.length();
      if (IFN[Len-3] == '.' && IFN[Len-2] == 'l' && IFN[Len-1] == 'l') {
        // Source ends in .ll
        OutputFilename = std::string(IFN.begin(), IFN.end()-3);
      } else {
        OutputFilename = IFN;   // Append a .bc to it
      }
      OutputFilename += ".bc";
    }
  }

  std::string ErrorInfo;
  OwningPtr<tool_output_file> Out
  (new tool_output_file(OutputFilename.c_str(), ErrorInfo,
                        raw_fd_ostream::F_Binary));
  if (!ErrorInfo.empty()) {
    errs() << ErrorInfo << '\n';
    exit(1);
  }

  if (Force || !CheckBitcodeOutputToConsole(Out->os(), true))
    WriteBitcodeToFile(M, Out->os());

  // Declare success.
  Out->keep();
}

int main(int argc, char **argv) {
  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  LLVMContext &Context = getGlobalContext();
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
  cl::ParseCommandLineOptions(argc, argv, "llvm .ll -> .bc assembler\n");

  // Parse the file now...
  SMDiagnostic Err;
  std::auto_ptr<Module> M(ParseAssemblyFile(InputFilename, Err, Context));
  if (M.get() == 0) {
    Err.print(argv[0], errs());
    return 1;
  }

  if (!DisableVerify) {
    std::string Err;
    if (verifyModule(*M.get(), ReturnStatusAction, &Err)) {
      errs() << argv[0]
             << ": assembly parsed, but does not verify as correct!\n";
      errs() << Err;
      return 1;
    }
  }

  if (DumpAsm) errs() << "Here's the assembly:\n" << *M.get();

  if (!DisableOutput)
    WriteOutputFile(M.get());

  return 0;
}