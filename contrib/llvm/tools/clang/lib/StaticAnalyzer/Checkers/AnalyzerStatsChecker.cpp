
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
#define DEBUG_TYPE "StatsChecker"

#include "ClangSACheckers.h"
#include "clang/AST/DeclObjC.h"
#include "clang/Basic/SourceManager.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExplodedGraph.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ento;

STATISTIC(NumBlocks,
          "The # of blocks in top level functions");
STATISTIC(NumBlocksUnreachable,
          "The # of unreachable blocks in analyzing top level functions");

namespace {
class AnalyzerStatsChecker : public Checker<check::EndAnalysis> {
public:
  void checkEndAnalysis(ExplodedGraph &G, BugReporter &B,ExprEngine &Eng) const;
};
}

void AnalyzerStatsChecker::checkEndAnalysis(ExplodedGraph &G,
                                            BugReporter &B,
                                            ExprEngine &Eng) const {
  const CFG *C  = 0;
  const SourceManager &SM = B.getSourceManager();
  llvm::SmallPtrSet<const CFGBlock*, 256> reachable;

  // Root node should have the location context of the top most function.
  const ExplodedNode *GraphRoot = *G.roots_begin();
  const LocationContext *LC = GraphRoot->getLocation().getLocationContext();

  const Decl *D = LC->getDecl();

  // Iterate over the exploded graph.
  for (ExplodedGraph::node_iterator I = G.nodes_begin();
      I != G.nodes_end(); ++I) {
    const ProgramPoint &P = I->getLocation();

    // Only check the coverage in the top level function (optimization).
    if (D != P.getLocationContext()->getDecl())
      continue;

    if (Optional<BlockEntrance> BE = P.getAs<BlockEntrance>()) {
      const CFGBlock *CB = BE->getBlock();
      reachable.insert(CB);
    }
  }

  // Get the CFG and the Decl of this block.
  C = LC->getCFG();

  unsigned total = 0, unreachable = 0;

  // Find CFGBlocks that were not covered by any node
  for (CFG::const_iterator I = C->begin(); I != C->end(); ++I) {
    const CFGBlock *CB = *I;
    ++total;
    // Check if the block is unreachable
    if (!reachable.count(CB)) {
      ++unreachable;
    }
  }

  // We never 'reach' the entry block, so correct the unreachable count
  unreachable--;
  // There is no BlockEntrance corresponding to the exit block as well, so
  // assume it is reached as well.
  unreachable--;

  // Generate the warning string
  SmallString<128> buf;
  llvm::raw_svector_ostream output(buf);
  PresumedLoc Loc = SM.getPresumedLoc(D->getLocation());
  if (!Loc.isValid())
    return;

  if (isa<FunctionDecl>(D) || isa<ObjCMethodDecl>(D)) {
    const NamedDecl *ND = cast<NamedDecl>(D);
    output << *ND;
  }
  else if (isa<BlockDecl>(D)) {
    output << "block(line:" << Loc.getLine() << ":col:" << Loc.getColumn();
  }
  
  NumBlocksUnreachable += unreachable;
  NumBlocks += total;
  std::string NameOfRootFunction = output.str();

  output << " -> Total CFGBlocks: " << total << " | Unreachable CFGBlocks: "
      << unreachable << " | Exhausted Block: "
      << (Eng.wasBlocksExhausted() ? "yes" : "no")
      << " | Empty WorkList: "
      << (Eng.hasEmptyWorkList() ? "yes" : "no");

  B.EmitBasicReport(D, "Analyzer Statistics", "Internal Statistics",
                    output.str(), PathDiagnosticLocation(D, SM));

  // Emit warning for each block we bailed out on.
  typedef CoreEngine::BlocksExhausted::const_iterator ExhaustedIterator;
  const CoreEngine &CE = Eng.getCoreEngine();
  for (ExhaustedIterator I = CE.blocks_exhausted_begin(),
      E = CE.blocks_exhausted_end(); I != E; ++I) {
    const BlockEdge &BE =  I->first;
    const CFGBlock *Exit = BE.getDst();
    const CFGElement &CE = Exit->front();
    if (Optional<CFGStmt> CS = CE.getAs<CFGStmt>()) {
      SmallString<128> bufI;
      llvm::raw_svector_ostream outputI(bufI);
      outputI << "(" << NameOfRootFunction << ")" <<
                 ": The analyzer generated a sink at this point";
      B.EmitBasicReport(
          D, "Sink Point", "Internal Statistics", outputI.str(),
          PathDiagnosticLocation::createBegin(CS->getStmt(), SM, LC));
    }
  }
}

void ento::registerAnalyzerStatsChecker(CheckerManager &mgr) {
  mgr.registerChecker<AnalyzerStatsChecker>();
}