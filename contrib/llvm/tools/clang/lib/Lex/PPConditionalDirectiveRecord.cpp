
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
#include "clang/Lex/PPConditionalDirectiveRecord.h"
#include "llvm/Support/Capacity.h"

using namespace clang;

PPConditionalDirectiveRecord::PPConditionalDirectiveRecord(SourceManager &SM)
  : SourceMgr(SM) {
  CondDirectiveStack.push_back(SourceLocation());
}

bool PPConditionalDirectiveRecord::rangeIntersectsConditionalDirective(
                                                      SourceRange Range) const {
  if (Range.isInvalid())
    return false;

  CondDirectiveLocsTy::const_iterator
    low = std::lower_bound(CondDirectiveLocs.begin(), CondDirectiveLocs.end(),
                           Range.getBegin(), CondDirectiveLoc::Comp(SourceMgr));
  if (low == CondDirectiveLocs.end())
    return false;

  if (SourceMgr.isBeforeInTranslationUnit(Range.getEnd(), low->getLoc()))
    return false;

  CondDirectiveLocsTy::const_iterator
    upp = std::upper_bound(low, CondDirectiveLocs.end(),
                           Range.getEnd(), CondDirectiveLoc::Comp(SourceMgr));
  SourceLocation uppRegion;
  if (upp != CondDirectiveLocs.end())
    uppRegion = upp->getRegionLoc();

  return low->getRegionLoc() != uppRegion;
}

SourceLocation PPConditionalDirectiveRecord::findConditionalDirectiveRegionLoc(
                                                     SourceLocation Loc) const {
  if (Loc.isInvalid())
    return SourceLocation();
  if (CondDirectiveLocs.empty())
    return SourceLocation();

  if (SourceMgr.isBeforeInTranslationUnit(CondDirectiveLocs.back().getLoc(),
                                          Loc))
    return CondDirectiveStack.back();

  CondDirectiveLocsTy::const_iterator
    low = std::lower_bound(CondDirectiveLocs.begin(), CondDirectiveLocs.end(),
                           Loc, CondDirectiveLoc::Comp(SourceMgr));
  assert(low != CondDirectiveLocs.end());
  return low->getRegionLoc();
}

void PPConditionalDirectiveRecord::addCondDirectiveLoc(
                                                      CondDirectiveLoc DirLoc) {
  // Ignore directives in system headers.
  if (SourceMgr.isInSystemHeader(DirLoc.getLoc()))
    return;

  assert(CondDirectiveLocs.empty() ||
         SourceMgr.isBeforeInTranslationUnit(CondDirectiveLocs.back().getLoc(),
                                             DirLoc.getLoc()));
  CondDirectiveLocs.push_back(DirLoc);
}

void PPConditionalDirectiveRecord::If(SourceLocation Loc,
                                      SourceRange ConditionRange) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.push_back(Loc);
}

void PPConditionalDirectiveRecord::Ifdef(SourceLocation Loc,
                                         const Token &MacroNameTok,
                                         const MacroDirective *MD) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.push_back(Loc);
}

void PPConditionalDirectiveRecord::Ifndef(SourceLocation Loc,
                                          const Token &MacroNameTok,
                                          const MacroDirective *MD) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.push_back(Loc);
}

void PPConditionalDirectiveRecord::Elif(SourceLocation Loc,
                                        SourceRange ConditionRange,
                                        SourceLocation IfLoc) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.back() = Loc;
}

void PPConditionalDirectiveRecord::Else(SourceLocation Loc,
                                        SourceLocation IfLoc) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.back() = Loc;
}

void PPConditionalDirectiveRecord::Endif(SourceLocation Loc,
                                         SourceLocation IfLoc) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  assert(!CondDirectiveStack.empty());
  CondDirectiveStack.pop_back();
}

size_t PPConditionalDirectiveRecord::getTotalMemory() const {
  return llvm::capacity_in_bytes(CondDirectiveLocs);
}