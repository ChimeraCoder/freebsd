
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

#include "clang/Frontend/Utils.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
namespace DOT = llvm::DOT;

namespace {
class DependencyGraphCallback : public PPCallbacks {
  const Preprocessor *PP;
  std::string OutputFile;
  std::string SysRoot;
  llvm::SetVector<const FileEntry *> AllFiles;
  typedef llvm::DenseMap<const FileEntry *,
                         SmallVector<const FileEntry *, 2> > DependencyMap;
  
  DependencyMap Dependencies;
  
private:
  raw_ostream &writeNodeReference(raw_ostream &OS,
                                  const FileEntry *Node);
  void OutputGraphFile();

public:
  DependencyGraphCallback(const Preprocessor *_PP, StringRef OutputFile,
                          StringRef SysRoot)
    : PP(_PP), OutputFile(OutputFile.str()), SysRoot(SysRoot.str()) { }

  virtual void InclusionDirective(SourceLocation HashLoc,
                                  const Token &IncludeTok,
                                  StringRef FileName,
                                  bool IsAngled,
                                  CharSourceRange FilenameRange,
                                  const FileEntry *File,
                                  StringRef SearchPath,
                                  StringRef RelativePath,
                                  const Module *Imported);

  virtual void EndOfMainFile() {
    OutputGraphFile();
  }
  
};
}

void clang::AttachDependencyGraphGen(Preprocessor &PP, StringRef OutputFile,
                                     StringRef SysRoot) {
  PP.addPPCallbacks(new DependencyGraphCallback(&PP, OutputFile, SysRoot));
}

void DependencyGraphCallback::InclusionDirective(SourceLocation HashLoc,
                                                 const Token &IncludeTok,
                                                 StringRef FileName,
                                                 bool IsAngled,
                                                 CharSourceRange FilenameRange,
                                                 const FileEntry *File,
                                                 StringRef SearchPath,
                                                 StringRef RelativePath,
                                                 const Module *Imported) {
  if (!File)
    return;
  
  SourceManager &SM = PP->getSourceManager();
  const FileEntry *FromFile
    = SM.getFileEntryForID(SM.getFileID(SM.getExpansionLoc(HashLoc)));
  if (FromFile == 0) 
    return;

  Dependencies[FromFile].push_back(File);
  
  AllFiles.insert(File);
  AllFiles.insert(FromFile);
}

raw_ostream &
DependencyGraphCallback::writeNodeReference(raw_ostream &OS,
                                            const FileEntry *Node) {
  OS << "header_" << Node->getUID();
  return OS;
}

void DependencyGraphCallback::OutputGraphFile() {
  std::string Err;
  llvm::raw_fd_ostream OS(OutputFile.c_str(), Err);
  if (!Err.empty()) {
    PP->getDiagnostics().Report(diag::err_fe_error_opening)
      << OutputFile << Err;
    return;
  }

  OS << "digraph \"dependencies\" {\n";
  
  // Write the nodes
  for (unsigned I = 0, N = AllFiles.size(); I != N; ++I) {
    // Write the node itself.
    OS.indent(2);
    writeNodeReference(OS, AllFiles[I]);
    OS << " [ shape=\"box\", label=\"";
    StringRef FileName = AllFiles[I]->getName();
    if (FileName.startswith(SysRoot))
      FileName = FileName.substr(SysRoot.size());
    
    OS << DOT::EscapeString(FileName)
    << "\"];\n";
  }

  // Write the edges
  for (DependencyMap::iterator F = Dependencies.begin(), 
                            FEnd = Dependencies.end();
       F != FEnd; ++F) {    
    for (unsigned I = 0, N = F->second.size(); I != N; ++I) {
      OS.indent(2);
      writeNodeReference(OS, F->first);
      OS << " -> ";
      writeNodeReference(OS, F->second[I]);
      OS << ";\n";
    }
  }
  OS << "}\n";
}