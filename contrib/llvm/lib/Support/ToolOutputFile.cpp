
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

#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/Signals.h"
using namespace llvm;

tool_output_file::CleanupInstaller::CleanupInstaller(const char *filename)
  : Filename(filename), Keep(false) {
  // Arrange for the file to be deleted if the process is killed.
  if (Filename != "-")
    sys::RemoveFileOnSignal(sys::Path(Filename));
}

tool_output_file::CleanupInstaller::~CleanupInstaller() {
  // Delete the file if the client hasn't told us not to.
  if (!Keep && Filename != "-")
    sys::Path(Filename).eraseFromDisk();

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  if (Filename != "-")
    sys::DontRemoveFileOnSignal(sys::Path(Filename));
}

tool_output_file::tool_output_file(const char *filename, std::string &ErrorInfo,
                                   unsigned Flags)
  : Installer(filename),
    OS(filename, ErrorInfo, Flags) {
  // If open fails, no cleanup is needed.
  if (!ErrorInfo.empty())
    Installer.Keep = true;
}