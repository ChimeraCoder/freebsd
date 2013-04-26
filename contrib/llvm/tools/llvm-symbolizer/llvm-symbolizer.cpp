
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

#include "LLVMSymbolize.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>
#include <cstring>
#include <string>

using namespace llvm;
using namespace symbolize;

static cl::opt<bool>
ClUseSymbolTable("use-symbol-table", cl::init(true),
                 cl::desc("Prefer names in symbol table to names "
                          "in debug info"));

static cl::opt<bool>
ClPrintFunctions("functions", cl::init(true),
                 cl::desc("Print function names as well as line "
                          "information for a given address"));

static cl::opt<bool>
ClPrintInlining("inlining", cl::init(true),
                cl::desc("Print all inlined frames for a given address"));

static cl::opt<bool>
ClDemangle("demangle", cl::init(true), cl::desc("Demangle function names"));

static bool parseCommand(bool &IsData, std::string &ModuleName,
                         uint64_t &ModuleOffset) {
  const char *kDataCmd = "DATA ";
  const char *kCodeCmd = "CODE ";
  const int kMaxInputStringLength = 1024;
  const char kDelimiters[] = " \n";
  char InputString[kMaxInputStringLength];
  if (!fgets(InputString, sizeof(InputString), stdin))
    return false;
  IsData = false;
  ModuleName = "";
  std::string ModuleOffsetStr = "";
  char *pos = InputString;
  if (strncmp(pos, kDataCmd, strlen(kDataCmd)) == 0) {
    IsData = true;
    pos += strlen(kDataCmd);
  } else if (strncmp(pos, kCodeCmd, strlen(kCodeCmd)) == 0) {
    IsData = false;
    pos += strlen(kCodeCmd);
  } else {
    // If no cmd, assume it's CODE.
    IsData = false;
  }
  // Skip delimiters and parse input filename.
  pos += strspn(pos, kDelimiters);
  if (*pos == '"' || *pos == '\'') {
    char quote = *pos;
    pos++;
    char *end = strchr(pos, quote);
    if (end == 0)
      return false;
    ModuleName = std::string(pos, end - pos);
    pos = end + 1;
  } else {
    int name_length = strcspn(pos, kDelimiters);
    ModuleName = std::string(pos, name_length);
    pos += name_length;
  }
  // Skip delimiters and parse module offset.
  pos += strspn(pos, kDelimiters);
  int offset_length = strcspn(pos, kDelimiters);
  ModuleOffsetStr = std::string(pos, offset_length);
  if (StringRef(ModuleOffsetStr).getAsInteger(0, ModuleOffset))
    return false;
  return true;
}

int main(int argc, char **argv) {
  // Print stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  llvm_shutdown_obj Y; // Call llvm_shutdown() on exit.

  cl::ParseCommandLineOptions(argc, argv, "llvm symbolizer for compiler-rt\n");
  LLVMSymbolizer::Options Opts(ClUseSymbolTable, ClPrintFunctions,
                               ClPrintInlining, ClDemangle);
  LLVMSymbolizer Symbolizer(Opts);

  bool IsData = false;
  std::string ModuleName;
  uint64_t ModuleOffset;
  while (parseCommand(IsData, ModuleName, ModuleOffset)) {
    std::string Result =
        IsData ? Symbolizer.symbolizeData(ModuleName, ModuleOffset)
               : Symbolizer.symbolizeCode(ModuleName, ModuleOffset);
    outs() << Result << "\n";
    outs().flush();
  }
  return 0;
}