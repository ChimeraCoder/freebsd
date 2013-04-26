
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
#include "clang/AST/Decl.h"
#include "clang/AST/DeclObjC.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"

using namespace clang;
using namespace ento;

namespace {
class NSAutoreleasePoolChecker
  : public Checker<check::PreObjCMessage> {
  mutable OwningPtr<BugType> BT;
  mutable Selector releaseS;

public:
  void checkPreObjCMessage(const ObjCMethodCall &msg, CheckerContext &C) const;
};

} // end anonymous namespace

void NSAutoreleasePoolChecker::checkPreObjCMessage(const ObjCMethodCall &msg,
                                                   CheckerContext &C) const {
  if (!msg.isInstanceMessage())
    return;

  const ObjCInterfaceDecl *OD = msg.getReceiverInterface();
  if (!OD)
    return;  
  if (!OD->getIdentifier()->isStr("NSAutoreleasePool"))
    return;

  if (releaseS.isNull())
    releaseS = GetNullarySelector("release", C.getASTContext());
  // Sending 'release' message?
  if (msg.getSelector() != releaseS)
    return;

  if (!BT)
    BT.reset(new BugType("Use -drain instead of -release",
                         "API Upgrade (Apple)"));

  ExplodedNode *N = C.addTransition();
  if (!N) {
    assert(0);
    return;
  }

  BugReport *Report = new BugReport(*BT, "Use -drain instead of -release when "
    "using NSAutoreleasePool and garbage collection", N);
  Report->addRange(msg.getSourceRange());
  C.emitReport(Report);
}

void ento::registerNSAutoreleasePoolChecker(CheckerManager &mgr) {
  if (mgr.getLangOpts().getGC() != LangOptions::NonGC)
    mgr.registerChecker<NSAutoreleasePoolChecker>();
}