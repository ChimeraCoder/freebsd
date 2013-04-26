
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
#include "ClangSACheckers.h"
#include "clang/AST/ParentMap.h"
#include "clang/AST/StmtObjC.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ento;

namespace {
class TraversalDumper : public Checker< check::BranchCondition,
                                        check::EndFunction > {
public:
  void checkBranchCondition(const Stmt *Condition, CheckerContext &C) const;
  void checkEndFunction(CheckerContext &C) const;
};
}

void TraversalDumper::checkBranchCondition(const Stmt *Condition,
                                           CheckerContext &C) const {
  // Special-case Objective-C's for-in loop, which uses the entire loop as its
  // condition. We just print the collection expression.
  const Stmt *Parent = dyn_cast<ObjCForCollectionStmt>(Condition);
  if (!Parent) {
    const ParentMap &Parents = C.getLocationContext()->getParentMap();
    Parent = Parents.getParent(Condition);
  }

  // It is mildly evil to print directly to llvm::outs() rather than emitting
  // warnings, but this ensures things do not get filtered out by the rest of
  // the static analyzer machinery.
  SourceLocation Loc = Parent->getLocStart();
  llvm::outs() << C.getSourceManager().getSpellingLineNumber(Loc) << " "
               << Parent->getStmtClassName() << "\n";
}

void TraversalDumper::checkEndFunction(CheckerContext &C) const {
  llvm::outs() << "--END FUNCTION--\n";
}

void ento::registerTraversalDumper(CheckerManager &mgr) {
  mgr.registerChecker<TraversalDumper>();
}

//------------------------------------------------------------------------------

namespace {
class CallDumper : public Checker< check::PreCall,
                                   check::PostCall > {
public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
};
}

void CallDumper::checkPreCall(const CallEvent &Call, CheckerContext &C) const {
  unsigned Indentation = 0;
  for (const LocationContext *LC = C.getLocationContext()->getParent();
       LC != 0; LC = LC->getParent())
    ++Indentation;

  // It is mildly evil to print directly to llvm::outs() rather than emitting
  // warnings, but this ensures things do not get filtered out by the rest of
  // the static analyzer machinery.
  llvm::outs().indent(Indentation);
  Call.dump(llvm::outs());
}

void CallDumper::checkPostCall(const CallEvent &Call, CheckerContext &C) const {
  const Expr *CallE = Call.getOriginExpr();
  if (!CallE)
    return;

  unsigned Indentation = 0;
  for (const LocationContext *LC = C.getLocationContext()->getParent();
       LC != 0; LC = LC->getParent())
    ++Indentation;

  // It is mildly evil to print directly to llvm::outs() rather than emitting
  // warnings, but this ensures things do not get filtered out by the rest of
  // the static analyzer machinery.
  llvm::outs().indent(Indentation);
  if (Call.getResultType()->isVoidType())
    llvm::outs() << "Returning void\n";
  else
    llvm::outs() << "Returning " << C.getSVal(CallE) << "\n";
}

void ento::registerCallDumper(CheckerManager &mgr) {
  mgr.registerChecker<CallDumper>();
}