
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

#include "DWARFAbbreviationDeclaration.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
using namespace dwarf;

bool
DWARFAbbreviationDeclaration::extract(DataExtractor data, uint32_t* offset_ptr){
  return extract(data, offset_ptr, data.getULEB128(offset_ptr));
}

bool
DWARFAbbreviationDeclaration::extract(DataExtractor data, uint32_t* offset_ptr,
                                      uint32_t code) {
  Code = code;
  Attribute.clear();
  if (Code) {
    Tag = data.getULEB128(offset_ptr);
    HasChildren = data.getU8(offset_ptr);

    while (data.isValidOffset(*offset_ptr)) {
      uint16_t attr = data.getULEB128(offset_ptr);
      uint16_t form = data.getULEB128(offset_ptr);

      if (attr && form)
        Attribute.push_back(DWARFAttribute(attr, form));
      else
        break;
    }

    return Tag != 0;
  } else {
    Tag = 0;
    HasChildren = false;
  }

  return false;
}

void DWARFAbbreviationDeclaration::dump(raw_ostream &OS) const {
  const char *tagString = TagString(getTag());
  OS << '[' << getCode() << "] ";
  if (tagString)
    OS << tagString;
  else
    OS << format("DW_TAG_Unknown_%x", getTag());
  OS << "\tDW_CHILDREN_" << (hasChildren() ? "yes" : "no") << '\n';
  for (unsigned i = 0, e = Attribute.size(); i != e; ++i) {
    OS << '\t';
    const char *attrString = AttributeString(Attribute[i].getAttribute());
    if (attrString)
      OS << attrString;
    else
      OS << format("DW_AT_Unknown_%x", Attribute[i].getAttribute());
    OS << '\t';
    const char *formString = FormEncodingString(Attribute[i].getForm());
    if (formString)
      OS << formString;
    else
      OS << format("DW_FORM_Unknown_%x", Attribute[i].getForm());
    OS << '\n';
  }
  OS << '\n';
}

uint32_t
DWARFAbbreviationDeclaration::findAttributeIndex(uint16_t attr) const {
  for (uint32_t i = 0, e = Attribute.size(); i != e; ++i) {
    if (Attribute[i].getAttribute() == attr)
      return i;
  }
  return -1U;
}