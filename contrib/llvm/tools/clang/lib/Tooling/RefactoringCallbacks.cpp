
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
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/RefactoringCallbacks.h"

namespace clang {
namespace tooling {

RefactoringCallback::RefactoringCallback() {}
tooling::Replacements &RefactoringCallback::getReplacements() {
  return Replace;
}

static Replacement replaceStmtWithText(SourceManager &Sources,
                                       const Stmt &From,
                                       StringRef Text) {
  return tooling::Replacement(Sources, CharSourceRange::getTokenRange(
      From.getSourceRange()), Text);
}
static Replacement replaceStmtWithStmt(SourceManager &Sources,
                                       const Stmt &From,
                                       const Stmt &To) {
  return replaceStmtWithText(Sources, From, Lexer::getSourceText(
      CharSourceRange::getTokenRange(To.getSourceRange()),
      Sources, LangOptions()));
}

ReplaceStmtWithText::ReplaceStmtWithText(StringRef FromId, StringRef ToText)
    : FromId(FromId), ToText(ToText) {}

void ReplaceStmtWithText::run(
    const ast_matchers::MatchFinder::MatchResult &Result) {
  if (const Stmt *FromMatch = Result.Nodes.getStmtAs<Stmt>(FromId)) {
    Replace.insert(tooling::Replacement(
        *Result.SourceManager,
        CharSourceRange::getTokenRange(FromMatch->getSourceRange()),
        ToText));
  }
}

ReplaceStmtWithStmt::ReplaceStmtWithStmt(StringRef FromId, StringRef ToId)
    : FromId(FromId), ToId(ToId) {}

void ReplaceStmtWithStmt::run(
    const ast_matchers::MatchFinder::MatchResult &Result) {
  const Stmt *FromMatch = Result.Nodes.getStmtAs<Stmt>(FromId);
  const Stmt *ToMatch = Result.Nodes.getStmtAs<Stmt>(ToId);
  if (FromMatch && ToMatch)
    Replace.insert(replaceStmtWithStmt(
        *Result.SourceManager, *FromMatch, *ToMatch));
}

ReplaceIfStmtWithItsBody::ReplaceIfStmtWithItsBody(StringRef Id,
                                                   bool PickTrueBranch)
    : Id(Id), PickTrueBranch(PickTrueBranch) {}

void ReplaceIfStmtWithItsBody::run(
    const ast_matchers::MatchFinder::MatchResult &Result) {
  if (const IfStmt *Node = Result.Nodes.getStmtAs<IfStmt>(Id)) {
    const Stmt *Body = PickTrueBranch ? Node->getThen() : Node->getElse();
    if (Body) {
      Replace.insert(replaceStmtWithStmt(*Result.SourceManager, *Node, *Body));
    } else if (!PickTrueBranch) {
      // If we want to use the 'else'-branch, but it doesn't exist, delete
      // the whole 'if'.
      Replace.insert(replaceStmtWithText(*Result.SourceManager, *Node, ""));
    }
  }
}

} // end namespace tooling
} // end namespace clang