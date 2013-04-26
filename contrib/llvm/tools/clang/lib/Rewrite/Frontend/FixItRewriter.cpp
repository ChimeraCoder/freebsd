
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

#include "clang/Rewrite/Frontend/FixItRewriter.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Edit/Commit.h"
#include "clang/Edit/EditsReceiver.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>

using namespace clang;

FixItRewriter::FixItRewriter(DiagnosticsEngine &Diags, SourceManager &SourceMgr,
                             const LangOptions &LangOpts,
                             FixItOptions *FixItOpts)
  : Diags(Diags),
    Editor(SourceMgr, LangOpts),
    Rewrite(SourceMgr, LangOpts),
    FixItOpts(FixItOpts),
    NumFailures(0),
    PrevDiagSilenced(false) {
  OwnsClient = Diags.ownsClient();
  Client = Diags.takeClient();
  Diags.setClient(this);
}

FixItRewriter::~FixItRewriter() {
  Diags.takeClient();
  Diags.setClient(Client, OwnsClient);
}

bool FixItRewriter::WriteFixedFile(FileID ID, raw_ostream &OS) {
  const RewriteBuffer *RewriteBuf = Rewrite.getRewriteBufferFor(ID);
  if (!RewriteBuf) return true;
  RewriteBuf->write(OS);
  OS.flush();
  return false;
}

namespace {

class RewritesReceiver : public edit::EditsReceiver {
  Rewriter &Rewrite;

public:
  RewritesReceiver(Rewriter &Rewrite) : Rewrite(Rewrite) { }

  virtual void insert(SourceLocation loc, StringRef text) {
    Rewrite.InsertText(loc, text);
  }
  virtual void replace(CharSourceRange range, StringRef text) {
    Rewrite.ReplaceText(range.getBegin(), Rewrite.getRangeSize(range), text);
  }
};

}

bool FixItRewriter::WriteFixedFiles(
            std::vector<std::pair<std::string, std::string> > *RewrittenFiles) {
  if (NumFailures > 0 && !FixItOpts->FixWhatYouCan) {
    Diag(FullSourceLoc(), diag::warn_fixit_no_changes);
    return true;
  }

  RewritesReceiver Rec(Rewrite);
  Editor.applyRewrites(Rec);

  for (iterator I = buffer_begin(), E = buffer_end(); I != E; ++I) {
    const FileEntry *Entry = Rewrite.getSourceMgr().getFileEntryForID(I->first);
    int fd;
    std::string Filename = FixItOpts->RewriteFilename(Entry->getName(), fd);
    std::string Err;
    OwningPtr<llvm::raw_fd_ostream> OS;
    if (fd != -1) {
      OS.reset(new llvm::raw_fd_ostream(fd, /*shouldClose=*/true));
    } else {
      OS.reset(new llvm::raw_fd_ostream(Filename.c_str(), Err,
                                        llvm::raw_fd_ostream::F_Binary));
    }
    if (!Err.empty()) {
      Diags.Report(clang::diag::err_fe_unable_to_open_output)
          << Filename << Err;
      continue;
    }
    RewriteBuffer &RewriteBuf = I->second;
    RewriteBuf.write(*OS);
    OS->flush();

    if (RewrittenFiles)
      RewrittenFiles->push_back(std::make_pair(Entry->getName(), Filename));
  }

  return false;
}

bool FixItRewriter::IncludeInDiagnosticCounts() const {
  return Client ? Client->IncludeInDiagnosticCounts() : true;
}

void FixItRewriter::HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                                     const Diagnostic &Info) {
  // Default implementation (Warnings/errors count).
  DiagnosticConsumer::HandleDiagnostic(DiagLevel, Info);

  if (!FixItOpts->Silent ||
      DiagLevel >= DiagnosticsEngine::Error ||
      (DiagLevel == DiagnosticsEngine::Note && !PrevDiagSilenced) ||
      (DiagLevel > DiagnosticsEngine::Note && Info.getNumFixItHints())) {
    Client->HandleDiagnostic(DiagLevel, Info);
    PrevDiagSilenced = false;
  } else {
    PrevDiagSilenced = true;
  }

  // Skip over any diagnostics that are ignored or notes.
  if (DiagLevel <= DiagnosticsEngine::Note)
    return;
  // Skip over errors if we are only fixing warnings.
  if (DiagLevel >= DiagnosticsEngine::Error && FixItOpts->FixOnlyWarnings) {
    ++NumFailures;
    return;
  }

  // Make sure that we can perform all of the modifications we
  // in this diagnostic.
  edit::Commit commit(Editor);
  for (unsigned Idx = 0, Last = Info.getNumFixItHints();
       Idx < Last; ++Idx) {
    const FixItHint &Hint = Info.getFixItHint(Idx);

    if (Hint.CodeToInsert.empty()) {
      if (Hint.InsertFromRange.isValid())
        commit.insertFromRange(Hint.RemoveRange.getBegin(),
                           Hint.InsertFromRange, /*afterToken=*/false,
                           Hint.BeforePreviousInsertions);
      else
        commit.remove(Hint.RemoveRange);
    } else {
      if (Hint.RemoveRange.isTokenRange() ||
          Hint.RemoveRange.getBegin() != Hint.RemoveRange.getEnd())
        commit.replace(Hint.RemoveRange, Hint.CodeToInsert);
      else
        commit.insert(Hint.RemoveRange.getBegin(), Hint.CodeToInsert,
                    /*afterToken=*/false, Hint.BeforePreviousInsertions);
    }
  }
  bool CanRewrite = Info.getNumFixItHints() > 0 && commit.isCommitable();

  if (!CanRewrite) {
    if (Info.getNumFixItHints() > 0)
      Diag(Info.getLocation(), diag::note_fixit_in_macro);

    // If this was an error, refuse to perform any rewriting.
    if (DiagLevel >= DiagnosticsEngine::Error) {
      if (++NumFailures == 1)
        Diag(Info.getLocation(), diag::note_fixit_unfixed_error);
    }
    return;
  }
  
  if (!Editor.commit(commit)) {
    ++NumFailures;
    Diag(Info.getLocation(), diag::note_fixit_failed);
    return;
  }

  Diag(Info.getLocation(), diag::note_fixit_applied);
}

/// \brief Emit a diagnostic via the adapted diagnostic client.
void FixItRewriter::Diag(SourceLocation Loc, unsigned DiagID) {
  // When producing this diagnostic, we temporarily bypass ourselves,
  // clear out any current diagnostic, and let the downstream client
  // format the diagnostic.
  Diags.takeClient();
  Diags.setClient(Client);
  Diags.Clear();
  Diags.Report(Loc, DiagID);
  Diags.takeClient();
  Diags.setClient(this);
}

DiagnosticConsumer *FixItRewriter::clone(DiagnosticsEngine &Diags) const {
  return new FixItRewriter(Diags, Diags.getSourceManager(), 
                           Rewrite.getLangOpts(), FixItOpts);
}

FixItOptions::~FixItOptions() {}