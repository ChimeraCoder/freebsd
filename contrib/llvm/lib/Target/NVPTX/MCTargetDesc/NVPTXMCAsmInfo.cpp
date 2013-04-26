
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

#include "NVPTXMCAsmInfo.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

bool CompileForDebugging;

// -debug-compile - Command line option to inform opt and llc passes to
// compile for debugging
static cl::opt<bool, true>
Debug("debug-compile", cl::desc("Compile for debugging"), cl::Hidden,
      cl::location(CompileForDebugging), cl::init(false));

void NVPTXMCAsmInfo::anchor() {}

NVPTXMCAsmInfo::NVPTXMCAsmInfo(const Target &T, const StringRef &TT) {
  Triple TheTriple(TT);
  if (TheTriple.getArch() == Triple::nvptx64) {
    PointerSize = CalleeSaveStackSlotSize = 8;
  }

  CommentString = "//";

  PrivateGlobalPrefix = "$L__";

  AllowPeriodsInName = false;

  HasSetDirective = false;

  HasSingleParameterDotFile = false;

  InlineAsmStart = " inline asm";
  InlineAsmEnd = " inline asm";

  SupportsDebugInformation = CompileForDebugging;
  HasDotTypeDotSizeDirective = false;

  Data8bitsDirective = " .b8 ";
  Data16bitsDirective = " .b16 ";
  Data32bitsDirective = " .b32 ";
  Data64bitsDirective = " .b64 ";
  PrivateGlobalPrefix = "";
  ZeroDirective = " .b8";
  AsciiDirective = " .b8";
  AscizDirective = " .b8";

  // @TODO: Can we just disable this?
  GlobalDirective = "\t// .globl\t";
}