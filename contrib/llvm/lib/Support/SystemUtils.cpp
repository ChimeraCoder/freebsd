
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

#include "llvm/Support/SystemUtils.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

bool llvm::CheckBitcodeOutputToConsole(raw_ostream &stream_to_check,
                                       bool print_warning) {
  if (stream_to_check.is_displayed()) {
    if (print_warning) {
      errs() << "WARNING: You're attempting to print out a bitcode file.\n"
                "This is inadvisable as it may cause display problems. If\n"
                "you REALLY want to taste LLVM bitcode first-hand, you\n"
                "can force output with the `-f' option.\n\n";
    }
    return true;
  }
  return false;
}

/// PrependMainExecutablePath - Prepend the path to the program being executed
/// to \p ExeName, given the value of argv[0] and the address of main()
/// itself. This allows us to find another LLVM tool if it is built in the same
/// directory. An empty string is returned on error; note that this function
/// just mainpulates the path and doesn't check for executability.
/// @brief Find a named executable.
sys::Path llvm::PrependMainExecutablePath(const std::string &ExeName,
                                          const char *Argv0, void *MainAddr) {
  // Check the directory that the calling program is in.  We can do
  // this if ProgramPath contains at least one / character, indicating that it
  // is a relative path to the executable itself.
  sys::Path Result = sys::Path::GetMainExecutable(Argv0, MainAddr);
  Result.eraseComponent();

  if (!Result.isEmpty()) {
    Result.appendComponent(ExeName);
    Result.appendSuffix(sys::Path::GetEXESuffix());
  }

  return Result;
}