
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

#include "llvm/Analysis/LibCallSemantics.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Function.h"
using namespace llvm;

/// getMap - This impl pointer in ~LibCallInfo is actually a StringMap.  This
/// helper does the cast.
static StringMap<const LibCallFunctionInfo*> *getMap(void *Ptr) {
  return static_cast<StringMap<const LibCallFunctionInfo*> *>(Ptr);
}

LibCallInfo::~LibCallInfo() {
  delete getMap(Impl);
}

const LibCallLocationInfo &LibCallInfo::getLocationInfo(unsigned LocID) const {
  // Get location info on the first call.
  if (NumLocations == 0)
    NumLocations = getLocationInfo(Locations);
  
  assert(LocID < NumLocations && "Invalid location ID!");
  return Locations[LocID];
}


/// getFunctionInfo - Return the LibCallFunctionInfo object corresponding to
/// the specified function if we have it.  If not, return null.
const LibCallFunctionInfo *
LibCallInfo::getFunctionInfo(const Function *F) const {
  StringMap<const LibCallFunctionInfo*> *Map = getMap(Impl);
  
  /// If this is the first time we are querying for this info, lazily construct
  /// the StringMap to index it.
  if (Map == 0) {
    Impl = Map = new StringMap<const LibCallFunctionInfo*>();
    
    const LibCallFunctionInfo *Array = getFunctionInfoArray();
    if (Array == 0) return 0;
    
    // We now have the array of entries.  Populate the StringMap.
    for (unsigned i = 0; Array[i].Name; ++i)
      (*Map)[Array[i].Name] = Array+i;
  }
  
  // Look up this function in the string map.
  return Map->lookup(F->getName());
}