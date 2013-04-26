
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

#define DEBUG_TYPE "ctags-emitter"

#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <algorithm>
#include <string>
#include <vector>
using namespace llvm;

namespace llvm { extern SourceMgr SrcMgr; }

namespace {

class Tag {
private:
  const std::string *Id;
  SMLoc Loc;
public:
  Tag(const std::string &Name, const SMLoc Location)
      : Id(&Name), Loc(Location) {}
  int operator<(const Tag &B) const { return *Id < *B.Id; }
  void emit(raw_ostream &OS) const {
    int BufferID = SrcMgr.FindBufferContainingLoc(Loc);
    MemoryBuffer *CurMB = SrcMgr.getBufferInfo(BufferID).Buffer;
    const char *BufferName = CurMB->getBufferIdentifier();
    std::pair<unsigned, unsigned> LineAndColumn = SrcMgr.getLineAndColumn(Loc);
    OS << *Id << "\t" << BufferName << "\t" << LineAndColumn.first << "\n";
  }
};

class CTagsEmitter {
private:
  RecordKeeper &Records;
public:
  CTagsEmitter(RecordKeeper &R) : Records(R) {}

  void run(raw_ostream &OS);

private:
  static SMLoc locate(const Record *R);
};

} // End anonymous namespace.

SMLoc CTagsEmitter::locate(const Record *R) {
  ArrayRef<SMLoc> Locs = R->getLoc();
  if (Locs.empty()) {
    SMLoc NullLoc;
    return NullLoc;
  }
  return Locs.front();
}

void CTagsEmitter::run(raw_ostream &OS) {
  const std::map<std::string, Record *> &Classes = Records.getClasses();
  const std::map<std::string, Record *> &Defs = Records.getDefs();
  std::vector<Tag> Tags;
  // Collect tags.
  Tags.reserve(Classes.size() + Defs.size());
  for (std::map<std::string, Record *>::const_iterator I = Classes.begin(),
                                                       E = Classes.end();
       I != E; ++I)
    Tags.push_back(Tag(I->first, locate(I->second)));
  for (std::map<std::string, Record *>::const_iterator I = Defs.begin(),
                                                       E = Defs.end();
       I != E; ++I)
    Tags.push_back(Tag(I->first, locate(I->second)));
  // Emit tags.
  std::sort(Tags.begin(), Tags.end());
  OS << "!_TAG_FILE_FORMAT\t1\t/original ctags format/\n";
  OS << "!_TAG_FILE_SORTED\t1\t/0=unsorted, 1=sorted, 2=foldcase/\n";
  for (std::vector<Tag>::const_iterator I = Tags.begin(), E = Tags.end();
       I != E; ++I)
    I->emit(OS);
}

namespace llvm {

void EmitCTags(RecordKeeper &RK, raw_ostream &OS) { CTagsEmitter(RK).run(OS); }

} // End llvm namespace.