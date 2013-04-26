
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

#include "llvm/TableGen/Error.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>

namespace llvm {

SourceMgr SrcMgr;
unsigned ErrorsPrinted = 0;

static void PrintMessage(ArrayRef<SMLoc> Loc, SourceMgr::DiagKind Kind,
                         const Twine &Msg) {
  // Count the total number of errors printed.
  // This is used to exit with an error code if there were any errors.
  if (Kind == SourceMgr::DK_Error)
    ++ErrorsPrinted;

  SMLoc NullLoc;
  if (Loc.empty())
    Loc = NullLoc;
  SrcMgr.PrintMessage(Loc.front(), Kind, Msg);
  for (unsigned i = 1; i < Loc.size(); ++i)
    SrcMgr.PrintMessage(Loc[i], SourceMgr::DK_Note,
                        "instantiated from multiclass");
}

void PrintWarning(ArrayRef<SMLoc> WarningLoc, const Twine &Msg) {
  PrintMessage(WarningLoc, SourceMgr::DK_Warning, Msg);
}

void PrintWarning(const char *Loc, const Twine &Msg) {
  SrcMgr.PrintMessage(SMLoc::getFromPointer(Loc), SourceMgr::DK_Warning, Msg);
}

void PrintWarning(const Twine &Msg) {
  errs() << "warning:" << Msg << "\n";
}

void PrintError(ArrayRef<SMLoc> ErrorLoc, const Twine &Msg) {
  PrintMessage(ErrorLoc, SourceMgr::DK_Error, Msg);
}

void PrintError(const char *Loc, const Twine &Msg) {
  SrcMgr.PrintMessage(SMLoc::getFromPointer(Loc), SourceMgr::DK_Error, Msg);
}

void PrintError(const Twine &Msg) {
  errs() << "error:" << Msg << "\n";
}

void PrintFatalError(const std::string &Msg) {
  PrintError(Twine(Msg));
  std::exit(1);
}

void PrintFatalError(ArrayRef<SMLoc> ErrorLoc, const std::string &Msg) {
  PrintError(ErrorLoc, Msg);
  std::exit(1);
}

} // end namespace llvm