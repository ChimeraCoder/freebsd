
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

#include "DWARFDebugRangeList.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

void DWARFDebugRangeList::clear() {
  Offset = -1U;
  AddressSize = 0;
  Entries.clear();
}

bool DWARFDebugRangeList::extract(DataExtractor data, uint32_t *offset_ptr) {
  clear();
  if (!data.isValidOffset(*offset_ptr))
    return false;
  AddressSize = data.getAddressSize();
  if (AddressSize != 4 && AddressSize != 8)
    return false;
  Offset = *offset_ptr;
  while (true) {
    RangeListEntry entry;
    uint32_t prev_offset = *offset_ptr;
    entry.StartAddress = data.getAddress(offset_ptr);
    entry.EndAddress = data.getAddress(offset_ptr);
    // Check that both values were extracted correctly.
    if (*offset_ptr != prev_offset + 2 * AddressSize) {
      clear();
      return false;
    }
    if (entry.isEndOfListEntry())
      break;
    Entries.push_back(entry);
  }
  return true;
}

void DWARFDebugRangeList::dump(raw_ostream &OS) const {
  for (int i = 0, n = Entries.size(); i != n; ++i) {
    const char *format_str = (AddressSize == 4
                              ? "%08x %08"  PRIx64 " %08"  PRIx64 "\n"
                              : "%08x %016" PRIx64 " %016" PRIx64 "\n");
    OS << format(format_str, Offset, Entries[i].StartAddress,
                                     Entries[i].EndAddress);
  }
  OS << format("%08x <End of list>\n", Offset);
}

bool DWARFDebugRangeList::containsAddress(uint64_t BaseAddress,
                                          uint64_t Address) const {
  for (int i = 0, n = Entries.size(); i != n; ++i) {
    if (Entries[i].isBaseAddressSelectionEntry(AddressSize))
      BaseAddress = Entries[i].EndAddress;
    else if (Entries[i].containsAddress(BaseAddress, Address))
      return true;
  }
  return false;
}