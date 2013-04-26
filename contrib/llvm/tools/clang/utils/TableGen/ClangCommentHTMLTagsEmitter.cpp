
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

#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/StringMatcher.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <vector>

using namespace llvm;

namespace clang {
void EmitClangCommentHTMLTags(RecordKeeper &Records, raw_ostream &OS) {
  std::vector<Record *> Tags = Records.getAllDerivedDefinitions("Tag");
  std::vector<StringMatcher::StringPair> Matches;
  for (std::vector<Record *>::iterator I = Tags.begin(), E = Tags.end();
       I != E; ++I) {
    Record &Tag = **I;
    std::string Spelling = Tag.getValueAsString("Spelling");
    Matches.push_back(StringMatcher::StringPair(Spelling, "return true;"));
  }

  emitSourceFileHeader("HTML tag name matcher", OS);

  OS << "bool isHTMLTagName(StringRef Name) {\n";
  StringMatcher("Name", Matches, OS).Emit();
  OS << "  return false;\n"
     << "}\n\n";
}

void EmitClangCommentHTMLTagsProperties(RecordKeeper &Records,
                                        raw_ostream &OS) {
  std::vector<Record *> Tags = Records.getAllDerivedDefinitions("Tag");
  std::vector<StringMatcher::StringPair> MatchesEndTagOptional;
  std::vector<StringMatcher::StringPair> MatchesEndTagForbidden;
  for (std::vector<Record *>::iterator I = Tags.begin(), E = Tags.end();
       I != E; ++I) {
    Record &Tag = **I;
    std::string Spelling = Tag.getValueAsString("Spelling");
    StringMatcher::StringPair Match(Spelling, "return true;");
    if (Tag.getValueAsBit("EndTagOptional"))
      MatchesEndTagOptional.push_back(Match);
    if (Tag.getValueAsBit("EndTagForbidden"))
      MatchesEndTagForbidden.push_back(Match);
  }

  emitSourceFileHeader("HTML tag properties", OS);

  OS << "bool isHTMLEndTagOptional(StringRef Name) {\n";
  StringMatcher("Name", MatchesEndTagOptional, OS).Emit();
  OS << "  return false;\n"
     << "}\n\n";

  OS << "bool isHTMLEndTagForbidden(StringRef Name) {\n";
  StringMatcher("Name", MatchesEndTagForbidden, OS).Emit();
  OS << "  return false;\n"
     << "}\n\n";
}
} // end namespace clang