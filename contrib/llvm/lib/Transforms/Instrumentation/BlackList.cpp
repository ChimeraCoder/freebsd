
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

#include "llvm/Transforms/Utils/BlackList.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"
#include <string>
#include <utility>

namespace llvm {

BlackList::BlackList(const StringRef Path) {
  // Validate and open blacklist file.
  if (Path.empty()) return;
  OwningPtr<MemoryBuffer> File;
  if (error_code EC = MemoryBuffer::getFile(Path, File)) {
    report_fatal_error("Can't open blacklist file: " + Path + ": " +
                       EC.message());
  }

  // Iterate through each line in the blacklist file.
  SmallVector<StringRef, 16> Lines;
  SplitString(File.take()->getBuffer(), Lines, "\n\r");
  StringMap<std::string> Regexps;
  for (SmallVector<StringRef, 16>::iterator I = Lines.begin(), E = Lines.end();
       I != E; ++I) {
    // Ignore empty lines and lines starting with "#"
    if (I->empty() || I->startswith("#"))
      continue;
    // Get our prefix and unparsed regexp.
    std::pair<StringRef, StringRef> SplitLine = I->split(":");
    StringRef Prefix = SplitLine.first;
    std::string Regexp = SplitLine.second;
    if (Regexp.empty()) {
      // Missing ':' in the line.
      report_fatal_error("malformed blacklist line: " + SplitLine.first);
    }

    // Replace * with .*
    for (size_t pos = 0; (pos = Regexp.find("*", pos)) != std::string::npos;
         pos += strlen(".*")) {
      Regexp.replace(pos, strlen("*"), ".*");
    }

    // Check that the regexp is valid.
    Regex CheckRE(Regexp);
    std::string Error;
    if (!CheckRE.isValid(Error)) {
      report_fatal_error("malformed blacklist regex: " + SplitLine.second +
          ": " + Error);
    }

    // Add this regexp into the proper group by its prefix.
    if (!Regexps[Prefix].empty())
      Regexps[Prefix] += "|";
    Regexps[Prefix] += Regexp;
  }

  // Iterate through each of the prefixes, and create Regexs for them.
  for (StringMap<std::string>::const_iterator I = Regexps.begin(),
       E = Regexps.end(); I != E; ++I) {
    Entries[I->getKey()] = new Regex(I->getValue());
  }
}

bool BlackList::isIn(const Function &F) const {
  return isIn(*F.getParent()) || inSection("fun", F.getName());
}

bool BlackList::isIn(const GlobalVariable &G) const {
  return isIn(*G.getParent()) || inSection("global", G.getName());
}

bool BlackList::isIn(const Module &M) const {
  return inSection("src", M.getModuleIdentifier());
}

static StringRef GetGVTypeString(const GlobalVariable &G) {
  // Types of GlobalVariables are always pointer types.
  Type *GType = G.getType()->getElementType();
  // For now we support blacklisting struct types only.
  if (StructType *SGType = dyn_cast<StructType>(GType)) {
    if (!SGType->isLiteral())
      return SGType->getName();
  }
  return "<unknown type>";
}

bool BlackList::isInInit(const GlobalVariable &G) const {
  return (isIn(*G.getParent()) ||
          inSection("global-init", G.getName()) ||
          inSection("global-init-type", GetGVTypeString(G)));
}

bool BlackList::inSection(const StringRef Section,
                          const StringRef Query) const {
  StringMap<Regex*>::const_iterator I = Entries.find(Section);
  if (I == Entries.end()) return false;

  Regex *FunctionRegex = I->getValue();
  return FunctionRegex->match(Query);
}

}  // namespace llvm