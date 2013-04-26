
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
#include "clang/Frontend/ASTUnit.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTDiagnostic.h"
#include "clang/AST/ASTImporter.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"

using namespace clang;

ASTConsumer *ASTMergeAction::CreateASTConsumer(CompilerInstance &CI,
                                               StringRef InFile) {
  return AdaptedAction->CreateASTConsumer(CI, InFile);
}

bool ASTMergeAction::BeginSourceFileAction(CompilerInstance &CI,
                                           StringRef Filename) {
  // FIXME: This is a hack. We need a better way to communicate the
  // AST file, compiler instance, and file name than member variables
  // of FrontendAction.
  AdaptedAction->setCurrentInput(getCurrentInput(), takeCurrentASTUnit());
  AdaptedAction->setCompilerInstance(&CI);
  return AdaptedAction->BeginSourceFileAction(CI, Filename);
}

void ASTMergeAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  CI.getDiagnostics().getClient()->BeginSourceFile(
                                         CI.getASTContext().getLangOpts());
  CI.getDiagnostics().SetArgToStringFn(&FormatASTNodeDiagnosticArgument,
                                       &CI.getASTContext());
  IntrusiveRefCntPtr<DiagnosticIDs>
      DiagIDs(CI.getDiagnostics().getDiagnosticIDs());
  for (unsigned I = 0, N = ASTFiles.size(); I != N; ++I) {
    IntrusiveRefCntPtr<DiagnosticsEngine>
        Diags(new DiagnosticsEngine(DiagIDs, &CI.getDiagnosticOpts(),
                                    CI.getDiagnostics().getClient(),
                                    /*ShouldOwnClient=*/false));
    ASTUnit *Unit = ASTUnit::LoadFromASTFile(ASTFiles[I], Diags,
                                             CI.getFileSystemOpts(), false);
    if (!Unit)
      continue;

    ASTImporter Importer(CI.getASTContext(), 
                         CI.getFileManager(),
                         Unit->getASTContext(), 
                         Unit->getFileManager(),
                         /*MinimalImport=*/false);

    TranslationUnitDecl *TU = Unit->getASTContext().getTranslationUnitDecl();
    for (DeclContext::decl_iterator D = TU->decls_begin(), 
                                 DEnd = TU->decls_end();
         D != DEnd; ++D) {
      // Don't re-import __va_list_tag, __builtin_va_list.
      if (NamedDecl *ND = dyn_cast<NamedDecl>(*D))
        if (IdentifierInfo *II = ND->getIdentifier())
          if (II->isStr("__va_list_tag") || II->isStr("__builtin_va_list"))
            continue;
      
      Importer.Import(*D);
    }

    delete Unit;
  }

  AdaptedAction->ExecuteAction();
  CI.getDiagnostics().getClient()->EndSourceFile();
}

void ASTMergeAction::EndSourceFileAction() {
  return AdaptedAction->EndSourceFileAction();
}

ASTMergeAction::ASTMergeAction(FrontendAction *AdaptedAction,
                               ArrayRef<std::string> ASTFiles)
  : AdaptedAction(AdaptedAction), ASTFiles(ASTFiles.begin(), ASTFiles.end()) {
  assert(AdaptedAction && "ASTMergeAction needs an action to adapt");
}

ASTMergeAction::~ASTMergeAction() { 
  delete AdaptedAction;
}

bool ASTMergeAction::usesPreprocessorOnly() const {
  return AdaptedAction->usesPreprocessorOnly();
}

TranslationUnitKind ASTMergeAction::getTranslationUnitKind() {
  return AdaptedAction->getTranslationUnitKind();
}

bool ASTMergeAction::hasPCHSupport() const {
  return AdaptedAction->hasPCHSupport();
}

bool ASTMergeAction::hasASTFileSupport() const {
  return AdaptedAction->hasASTFileSupport();
}

bool ASTMergeAction::hasCodeCompletionSupport() const {
  return AdaptedAction->hasCodeCompletionSupport();
}