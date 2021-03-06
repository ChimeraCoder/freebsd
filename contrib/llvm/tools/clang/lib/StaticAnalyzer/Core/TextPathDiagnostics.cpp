
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

#include "clang/StaticAnalyzer/Core/PathDiagnosticConsumers.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/StaticAnalyzer/Core/BugReporter/PathDiagnostic.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;
using namespace ento;
using namespace llvm;

namespace {

/// \brief Simple path diagnostic client used for outputting as diagnostic notes
/// the sequence of events.
class TextPathDiagnostics : public PathDiagnosticConsumer {
  const std::string OutputFile;
  DiagnosticsEngine &Diag;

public:
  TextPathDiagnostics(const std::string& output, DiagnosticsEngine &diag)
    : OutputFile(output), Diag(diag) {}

  void FlushDiagnosticsImpl(std::vector<const PathDiagnostic *> &Diags,
                            FilesMade *filesMade);
  
  virtual StringRef getName() const {
    return "TextPathDiagnostics";
  }

  PathGenerationScheme getGenerationScheme() const { return Minimal; }
  bool supportsLogicalOpControlFlow() const { return true; }
  bool supportsAllBlockEdges() const { return true; }
  virtual bool supportsCrossFileDiagnostics() const { return true; }
};

} // end anonymous namespace

void ento::createTextPathDiagnosticConsumer(AnalyzerOptions &AnalyzerOpts,
                                            PathDiagnosticConsumers &C,
                                            const std::string& out,
                                            const Preprocessor &PP) {
  C.push_back(new TextPathDiagnostics(out, PP.getDiagnostics()));
}

void TextPathDiagnostics::FlushDiagnosticsImpl(
                              std::vector<const PathDiagnostic *> &Diags,
                              FilesMade *) {
  for (std::vector<const PathDiagnostic *>::iterator it = Diags.begin(),
       et = Diags.end(); it != et; ++it) {
    const PathDiagnostic *D = *it;

    PathPieces FlatPath = D->path.flatten(/*ShouldFlattenMacros=*/true);
    for (PathPieces::const_iterator I = FlatPath.begin(), E = FlatPath.end(); 
         I != E; ++I) {
      unsigned diagID =
        Diag.getDiagnosticIDs()->getCustomDiagID(DiagnosticIDs::Note,
                                                 (*I)->getString());
      Diag.Report((*I)->getLocation().asLocation(), diagID);
    }
  }
}