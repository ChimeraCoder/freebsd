
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

#include "clang/Frontend/Utils.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

namespace {
class HeaderIncludesCallback : public PPCallbacks {
  SourceManager &SM;
  raw_ostream *OutputFile;
  unsigned CurrentIncludeDepth;
  bool HasProcessedPredefines;
  bool OwnsOutputFile;
  bool ShowAllHeaders;
  bool ShowDepth;

public:
  HeaderIncludesCallback(const Preprocessor *PP, bool ShowAllHeaders_,
                         raw_ostream *OutputFile_, bool OwnsOutputFile_,
                         bool ShowDepth_)
    : SM(PP->getSourceManager()), OutputFile(OutputFile_),
      CurrentIncludeDepth(0), HasProcessedPredefines(false),
      OwnsOutputFile(OwnsOutputFile_), ShowAllHeaders(ShowAllHeaders_),
      ShowDepth(ShowDepth_) {}

  ~HeaderIncludesCallback() {
    if (OwnsOutputFile)
      delete OutputFile;
  }

  virtual void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                           SrcMgr::CharacteristicKind FileType,
                           FileID PrevFID);
};
}

void clang::AttachHeaderIncludeGen(Preprocessor &PP, bool ShowAllHeaders,
                                   StringRef OutputPath, bool ShowDepth) {
  raw_ostream *OutputFile = &llvm::errs();
  bool OwnsOutputFile = false;

  // Open the output file, if used.
  if (!OutputPath.empty()) {
    std::string Error;
    llvm::raw_fd_ostream *OS = new llvm::raw_fd_ostream(
      OutputPath.str().c_str(), Error, llvm::raw_fd_ostream::F_Append);
    if (!Error.empty()) {
      PP.getDiagnostics().Report(
        clang::diag::warn_fe_cc_print_header_failure) << Error;
      delete OS;
    } else {
      OS->SetUnbuffered();
      OS->SetUseAtomicWrites(true);
      OutputFile = OS;
      OwnsOutputFile = true;
    }
  }

  PP.addPPCallbacks(new HeaderIncludesCallback(&PP, ShowAllHeaders,
                                               OutputFile, OwnsOutputFile,
                                               ShowDepth));
}

void HeaderIncludesCallback::FileChanged(SourceLocation Loc,
                                         FileChangeReason Reason,
                                       SrcMgr::CharacteristicKind NewFileType,
                                       FileID PrevFID) {
  // Unless we are exiting a #include, make sure to skip ahead to the line the
  // #include directive was at.
  PresumedLoc UserLoc = SM.getPresumedLoc(Loc);
  if (UserLoc.isInvalid())
    return;

  // Adjust the current include depth.
  if (Reason == PPCallbacks::EnterFile) {
    ++CurrentIncludeDepth;
  } else if (Reason == PPCallbacks::ExitFile) {
    if (CurrentIncludeDepth)
      --CurrentIncludeDepth;

    // We track when we are done with the predefines by watching for the first
    // place where we drop back to a nesting depth of 1.
    if (CurrentIncludeDepth == 1 && !HasProcessedPredefines)
      HasProcessedPredefines = true;

    return;
  } else
    return;

  // Show the header if we are (a) past the predefines, or (b) showing all
  // headers and in the predefines at a depth past the initial file and command
  // line buffers.
  bool ShowHeader = (HasProcessedPredefines ||
                     (ShowAllHeaders && CurrentIncludeDepth > 2));

  // Dump the header include information we are past the predefines buffer or
  // are showing all headers.
  if (ShowHeader && Reason == PPCallbacks::EnterFile) {
    // Write to a temporary string to avoid unnecessary flushing on errs().
    SmallString<512> Filename(UserLoc.getFilename());
    Lexer::Stringify(Filename);

    SmallString<256> Msg;
    if (ShowDepth) {
      // The main source file is at depth 1, so skip one dot.
      for (unsigned i = 1; i != CurrentIncludeDepth; ++i)
        Msg += '.';
      Msg += ' ';
    }
    Msg += Filename;
    Msg += '\n';

    OutputFile->write(Msg.data(), Msg.size());
  }
}