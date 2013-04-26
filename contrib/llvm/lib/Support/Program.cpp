
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

#include "llvm/Support/Program.h"
#include "llvm/Config/config.h"
#include "llvm/Support/system_error.h"
using namespace llvm;
using namespace sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

int
Program::ExecuteAndWait(const Path& path,
                        const char** args,
                        const char** envp,
                        const Path** redirects,
                        unsigned secondsToWait,
                        unsigned memoryLimit,
                        std::string* ErrMsg,
                        bool *ExecutionFailed) {
  Program prg;
  if (prg.Execute(path, args, envp, redirects, memoryLimit, ErrMsg)) {
    if (ExecutionFailed) *ExecutionFailed = false;
    return prg.Wait(path, secondsToWait, ErrMsg);
  }
  if (ExecutionFailed) *ExecutionFailed = true;
  return -1;
}

void
Program::ExecuteNoWait(const Path& path,
                       const char** args,
                       const char** envp,
                       const Path** redirects,
                       unsigned memoryLimit,
                       std::string* ErrMsg) {
  Program prg;
  prg.Execute(path, args, envp, redirects, memoryLimit, ErrMsg);
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Program.inc"
#endif
#ifdef LLVM_ON_WIN32
#include "Windows/Program.inc"
#endif