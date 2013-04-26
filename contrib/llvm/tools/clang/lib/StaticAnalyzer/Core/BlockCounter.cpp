
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

#include "clang/StaticAnalyzer/Core/PathSensitive/BlockCounter.h"
#include "llvm/ADT/ImmutableMap.h"

using namespace clang;
using namespace ento;

namespace {

class CountKey {
  const StackFrameContext *CallSite;
  unsigned BlockID;

public:
  CountKey(const StackFrameContext *CS, unsigned ID) 
    : CallSite(CS), BlockID(ID) {}

  bool operator==(const CountKey &RHS) const {
    return (CallSite == RHS.CallSite) && (BlockID == RHS.BlockID);
  }

  bool operator<(const CountKey &RHS) const {
    return (CallSite == RHS.CallSite) ? (BlockID < RHS.BlockID) 
                                      : (CallSite < RHS.CallSite);
  }

  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddPointer(CallSite);
    ID.AddInteger(BlockID);
  }
};

}

typedef llvm::ImmutableMap<CountKey, unsigned> CountMap;

static inline CountMap GetMap(void *D) {
  return CountMap(static_cast<CountMap::TreeTy*>(D));
}

static inline CountMap::Factory& GetFactory(void *F) {
  return *static_cast<CountMap::Factory*>(F);
}

unsigned BlockCounter::getNumVisited(const StackFrameContext *CallSite, 
                                       unsigned BlockID) const {
  CountMap M = GetMap(Data);
  CountMap::data_type* T = M.lookup(CountKey(CallSite, BlockID));
  return T ? *T : 0;
}

BlockCounter::Factory::Factory(llvm::BumpPtrAllocator& Alloc) {
  F = new CountMap::Factory(Alloc);
}

BlockCounter::Factory::~Factory() {
  delete static_cast<CountMap::Factory*>(F);
}

BlockCounter
BlockCounter::Factory::IncrementCount(BlockCounter BC, 
                                        const StackFrameContext *CallSite,
                                        unsigned BlockID) {
  return BlockCounter(GetFactory(F).add(GetMap(BC.Data), 
                                          CountKey(CallSite, BlockID),
                             BC.getNumVisited(CallSite, BlockID)+1).getRoot());
}

BlockCounter
BlockCounter::Factory::GetEmptyCounter() {
  return BlockCounter(GetFactory(F).getEmptyMap().getRoot());
}