
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
#include "clang/Basic/Builtins.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {

class BuiltinFunctionChecker : public Checker<eval::Call> {
public:
  bool evalCall(const CallExpr *CE, CheckerContext &C) const;
};

}

bool BuiltinFunctionChecker::evalCall(const CallExpr *CE,
                                      CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  const FunctionDecl *FD = C.getCalleeDecl(CE);
  const LocationContext *LCtx = C.getLocationContext();
  if (!FD)
    return false;

  unsigned id = FD->getBuiltinID();

  if (!id)
    return false;

  switch (id) {
  case Builtin::BI__builtin_expect: {
    // For __builtin_expect, just return the value of the subexpression.
    assert (CE->arg_begin() != CE->arg_end());
    SVal X = state->getSVal(*(CE->arg_begin()), LCtx);
    C.addTransition(state->BindExpr(CE, LCtx, X));
    return true;
  }

  case Builtin::BI__builtin_alloca: {
    // FIXME: Refactor into StoreManager itself?
    MemRegionManager& RM = C.getStoreManager().getRegionManager();
    const AllocaRegion* R =
      RM.getAllocaRegion(CE, C.blockCount(), C.getLocationContext());

    // Set the extent of the region in bytes. This enables us to use the
    // SVal of the argument directly. If we save the extent in bits, we
    // cannot represent values like symbol*8.
    DefinedOrUnknownSVal Size =
        state->getSVal(*(CE->arg_begin()), LCtx).castAs<DefinedOrUnknownSVal>();

    SValBuilder& svalBuilder = C.getSValBuilder();
    DefinedOrUnknownSVal Extent = R->getExtent(svalBuilder);
    DefinedOrUnknownSVal extentMatchesSizeArg =
      svalBuilder.evalEQ(state, Extent, Size);
    state = state->assume(extentMatchesSizeArg, true);
    assert(state && "The region should not have any previous constraints");

    C.addTransition(state->BindExpr(CE, LCtx, loc::MemRegionVal(R)));
    return true;
  }
  }

  return false;
}

void ento::registerBuiltinFunctionChecker(CheckerManager &mgr) {
  mgr.registerChecker<BuiltinFunctionChecker>();
}