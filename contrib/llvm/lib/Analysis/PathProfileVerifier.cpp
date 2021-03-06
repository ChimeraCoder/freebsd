
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
#define DEBUG_TYPE "path-profile-verifier"

#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/PathProfileInfo.h"
#include "llvm/Analysis/ProfileInfoTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <stdio.h>

using namespace llvm;

namespace {
  class PathProfileVerifier : public ModulePass {
  private:
    bool runOnModule(Module &M);

  public:
    static char ID; // Pass identification, replacement for typeid
    PathProfileVerifier() : ModulePass(ID) {
      initializePathProfileVerifierPass(*PassRegistry::getPassRegistry());
    }


    virtual const char *getPassName() const {
      return "Path Profiler Verifier";
    }

    // The verifier requires the path profile and edge profile.
    virtual void getAnalysisUsage(AnalysisUsage& AU) const;
  };
}

static cl::opt<std::string>
EdgeProfileFilename("path-profile-verifier-file",
  cl::init("edgefrompath.llvmprof.out"),
  cl::value_desc("filename"),
  cl::desc("Edge profile file generated by -path-profile-verifier"),
  cl::Hidden);

char PathProfileVerifier::ID = 0;
INITIALIZE_PASS(PathProfileVerifier, "path-profile-verifier",
                "Compare the path profile derived edge profile against the "
                "edge profile.", true, true)

ModulePass *llvm::createPathProfileVerifierPass() {
  return new PathProfileVerifier();
}

// The verifier requires the path profile and edge profile.
void PathProfileVerifier::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<PathProfileInfo>();
  AU.addPreserved<PathProfileInfo>();
}

typedef std::map<unsigned, unsigned> DuplicateToIndexMap;
typedef std::map<BasicBlock*,DuplicateToIndexMap> BlockToDuplicateMap;
typedef std::map<BasicBlock*,BlockToDuplicateMap> NestedBlockToIndexMap;

// the verifier iterates through each path to gather the total
// number of edge frequencies
bool PathProfileVerifier::runOnModule (Module &M) {
  PathProfileInfo& pathProfileInfo = getAnalysis<PathProfileInfo>();

  // setup a data structure to map path edges which index an
  // array of edge counters
  NestedBlockToIndexMap arrayMap;
  unsigned i = 0;
  for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
    if (F->isDeclaration()) continue;

    arrayMap[(BasicBlock*)0][F->begin()][0] = i++;

    for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
      TerminatorInst *TI = BB->getTerminator();

      unsigned duplicate = 0;
      BasicBlock* prev = 0;
      for (unsigned s = 0, e = TI->getNumSuccessors(); s != e;
           prev = TI->getSuccessor(s), ++s) {
        if (prev == TI->getSuccessor(s))
          duplicate++;
        else duplicate = 0;

        arrayMap[BB][TI->getSuccessor(s)][duplicate] = i++;
      }
    }
  }

  std::vector<unsigned> edgeArray(i);

  // iterate through each path and increment the edge counters as needed
  for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
    if (F->isDeclaration()) continue;

    pathProfileInfo.setCurrentFunction(F);

    DEBUG(dbgs() << "function '" << F->getName() << "' ran "
          << pathProfileInfo.pathsRun()
          << "/" << pathProfileInfo.getPotentialPathCount()
          << " potential paths\n");

    for( ProfilePathIterator nextPath = pathProfileInfo.pathBegin(),
           endPath = pathProfileInfo.pathEnd();
         nextPath != endPath; nextPath++ ) {
      ProfilePath* currentPath = nextPath->second;

      ProfilePathEdgeVector* pev = currentPath->getPathEdges();
      DEBUG(dbgs () << "path #" << currentPath->getNumber() << ": "
            << currentPath->getCount() << "\n");
      // setup the entry edge (normally path profiling doesn't care about this)
      if (currentPath->getFirstBlockInPath() == &F->getEntryBlock())
        edgeArray[arrayMap[(BasicBlock*)0][currentPath->getFirstBlockInPath()][0]]
          += currentPath->getCount();

      for( ProfilePathEdgeIterator nextEdge = pev->begin(),
             endEdge = pev->end(); nextEdge != endEdge; nextEdge++ ) {
        if (nextEdge != pev->begin())
          DEBUG(dbgs() << " :: ");

        BasicBlock* source = nextEdge->getSource();
        BasicBlock* target = nextEdge->getTarget();
        unsigned duplicateNumber = nextEdge->getDuplicateNumber();
        DEBUG(dbgs() << source->getName() << " --{" << duplicateNumber
                     << "}--> " << target->getName());

        // Ensure all the referenced edges exist
        // TODO: make this a separate function
        if( !arrayMap.count(source) ) {
          errs() << "  error [" << F->getName() << "()]: source '"
                 << source->getName()
                 << "' does not exist in the array map.\n";
        } else if( !arrayMap[source].count(target) ) {
          errs() << "  error [" << F->getName() << "()]: target '"
                 << target->getName()
                 << "' does not exist in the array map.\n";
        } else if( !arrayMap[source][target].count(duplicateNumber) ) {
          errs() << "  error [" << F->getName() << "()]: edge "
                 << source->getName() << " -> " << target->getName()
                 << " duplicate number " << duplicateNumber
                 << " does not exist in the array map.\n";
        } else {
          edgeArray[arrayMap[source][target][duplicateNumber]]
            += currentPath->getCount();
        }
      }

      DEBUG(errs() << "\n");

      delete pev;
    }
  }

  std::string errorInfo;
  std::string filename = EdgeProfileFilename;

  // Open a handle to the file
  FILE* edgeFile = fopen(filename.c_str(),"wb");

  if (!edgeFile) {
    errs() << "error: unable to open file '" << filename << "' for output.\n";
    return false;
  }

  errs() << "Generating edge profile '" << filename << "' ...\n";

  // write argument info
  unsigned type = ArgumentInfo;
  unsigned num = pathProfileInfo.argList.size();
  int zeros = 0;

  fwrite(&type,sizeof(unsigned),1,edgeFile);
  fwrite(&num,sizeof(unsigned),1,edgeFile);
  fwrite(pathProfileInfo.argList.c_str(),1,num,edgeFile);
  if (num&3)
    fwrite(&zeros, 1, 4-(num&3), edgeFile);

  type = EdgeInfo;
  num = edgeArray.size();
  fwrite(&type,sizeof(unsigned),1,edgeFile);
  fwrite(&num,sizeof(unsigned),1,edgeFile);

  // write each edge to the file
  for( std::vector<unsigned>::iterator s = edgeArray.begin(),
         e = edgeArray.end(); s != e; s++)
    fwrite(&*s, sizeof (unsigned), 1, edgeFile);

  fclose (edgeFile);

  return true;
}