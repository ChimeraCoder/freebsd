
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

#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"

using namespace clang;
using namespace ento;

void AnalysisManager::anchor() { }

AnalysisManager::AnalysisManager(ASTContext &ctx, DiagnosticsEngine &diags,
                                 const LangOptions &lang,
                                 const PathDiagnosticConsumers &PDC,
                                 StoreManagerCreator storemgr,
                                 ConstraintManagerCreator constraintmgr, 
                                 CheckerManager *checkerMgr,
                                 AnalyzerOptions &Options)
  : AnaCtxMgr(Options.UnoptimizedCFG,
              /*AddImplicitDtors=*/true,
              /*AddInitializers=*/true,
              Options.includeTemporaryDtorsInCFG(),
              Options.shouldSynthesizeBodies(),
              Options.shouldConditionalizeStaticInitializers()),
    Ctx(ctx),
    Diags(diags),
    LangOpts(lang),
    PathConsumers(PDC),
    CreateStoreMgr(storemgr), CreateConstraintMgr(constraintmgr),
    CheckerMgr(checkerMgr),
    options(Options) {
  AnaCtxMgr.getCFGBuildOptions().setAllAlwaysAdd();
}

AnalysisManager::~AnalysisManager() {
  FlushDiagnostics();
  for (PathDiagnosticConsumers::iterator I = PathConsumers.begin(),
       E = PathConsumers.end(); I != E; ++I) {
    delete *I;
  }
}

void AnalysisManager::FlushDiagnostics() {
  PathDiagnosticConsumer::FilesMade filesMade;
  for (PathDiagnosticConsumers::iterator I = PathConsumers.begin(),
       E = PathConsumers.end();
       I != E; ++I) {
    (*I)->FlushDiagnostics(&filesMade);
  }
}