
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

#include "clang/Rewrite/Frontend/ASTConsumers.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Rewrite/Core/HTMLRewrite.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

//===----------------------------------------------------------------------===//
// Functional HTML pretty-printing.
//===----------------------------------------------------------------------===//

namespace {
  class HTMLPrinter : public ASTConsumer {
    Rewriter R;
    raw_ostream *Out;
    Preprocessor &PP;
    bool SyntaxHighlight, HighlightMacros;

  public:
    HTMLPrinter(raw_ostream *OS, Preprocessor &pp,
                bool _SyntaxHighlight, bool _HighlightMacros)
      : Out(OS), PP(pp), SyntaxHighlight(_SyntaxHighlight),
        HighlightMacros(_HighlightMacros) {}

    void Initialize(ASTContext &context);
    void HandleTranslationUnit(ASTContext &Ctx);
  };
}

ASTConsumer* clang::CreateHTMLPrinter(raw_ostream *OS,
                                      Preprocessor &PP,
                                      bool SyntaxHighlight,
                                      bool HighlightMacros) {
  return new HTMLPrinter(OS, PP, SyntaxHighlight, HighlightMacros);
}

void HTMLPrinter::Initialize(ASTContext &context) {
  R.setSourceMgr(context.getSourceManager(), context.getLangOpts());
}

void HTMLPrinter::HandleTranslationUnit(ASTContext &Ctx) {
  if (PP.getDiagnostics().hasErrorOccurred())
    return;

  // Format the file.
  FileID FID = R.getSourceMgr().getMainFileID();
  const FileEntry* Entry = R.getSourceMgr().getFileEntryForID(FID);
  const char* Name;
  // In some cases, in particular the case where the input is from stdin,
  // there is no entry.  Fall back to the memory buffer for a name in those
  // cases.
  if (Entry)
    Name = Entry->getName();
  else
    Name = R.getSourceMgr().getBuffer(FID)->getBufferIdentifier();

  html::AddLineNumbers(R, FID);
  html::AddHeaderFooterInternalBuiltinCSS(R, FID, Name);

  // If we have a preprocessor, relex the file and syntax highlight.
  // We might not have a preprocessor if we come from a deserialized AST file,
  // for example.

  if (SyntaxHighlight) html::SyntaxHighlight(R, FID, PP);
  if (HighlightMacros) html::HighlightMacros(R, FID, PP);
  html::EscapeText(R, FID, false, true);

  // Emit the HTML.
  const RewriteBuffer &RewriteBuf = R.getEditBuffer(FID);
  char *Buffer = (char*)malloc(RewriteBuf.size());
  std::copy(RewriteBuf.begin(), RewriteBuf.end(), Buffer);
  Out->write(Buffer, RewriteBuf.size());
  free(Buffer);
}