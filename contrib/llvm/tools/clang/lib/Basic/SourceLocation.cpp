
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

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/PrettyStackTrace.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>
using namespace clang;

//===----------------------------------------------------------------------===//
// PrettyStackTraceLoc
//===----------------------------------------------------------------------===//

void PrettyStackTraceLoc::print(raw_ostream &OS) const {
  if (Loc.isValid()) {
    Loc.print(OS, SM);
    OS << ": ";
  }
  OS << Message << '\n';
}

//===----------------------------------------------------------------------===//
// SourceLocation
//===----------------------------------------------------------------------===//

void SourceLocation::print(raw_ostream &OS, const SourceManager &SM)const{
  if (!isValid()) {
    OS << "<invalid loc>";
    return;
  }

  if (isFileID()) {
    PresumedLoc PLoc = SM.getPresumedLoc(*this);
    
    if (PLoc.isInvalid()) {
      OS << "<invalid>";
      return;
    }
    // The macro expansion and spelling pos is identical for file locs.
    OS << PLoc.getFilename() << ':' << PLoc.getLine()
       << ':' << PLoc.getColumn();
    return;
  }

  SM.getExpansionLoc(*this).print(OS, SM);

  OS << " <Spelling=";
  SM.getSpellingLoc(*this).print(OS, SM);
  OS << '>';
}

std::string SourceLocation::printToString(const SourceManager &SM) const {
  std::string S;
  llvm::raw_string_ostream OS(S);
  print(OS, SM);
  return OS.str();
}

void SourceLocation::dump(const SourceManager &SM) const {
  print(llvm::errs(), SM);
}

//===----------------------------------------------------------------------===//
// FullSourceLoc
//===----------------------------------------------------------------------===//

FileID FullSourceLoc::getFileID() const {
  assert(isValid());
  return SrcMgr->getFileID(*this);
}


FullSourceLoc FullSourceLoc::getExpansionLoc() const {
  assert(isValid());
  return FullSourceLoc(SrcMgr->getExpansionLoc(*this), *SrcMgr);
}

FullSourceLoc FullSourceLoc::getSpellingLoc() const {
  assert(isValid());
  return FullSourceLoc(SrcMgr->getSpellingLoc(*this), *SrcMgr);
}

unsigned FullSourceLoc::getExpansionLineNumber(bool *Invalid) const {
  assert(isValid());
  return SrcMgr->getExpansionLineNumber(*this, Invalid);
}

unsigned FullSourceLoc::getExpansionColumnNumber(bool *Invalid) const {
  assert(isValid());
  return SrcMgr->getExpansionColumnNumber(*this, Invalid);
}

unsigned FullSourceLoc::getSpellingLineNumber(bool *Invalid) const {
  assert(isValid());
  return SrcMgr->getSpellingLineNumber(*this, Invalid);
}

unsigned FullSourceLoc::getSpellingColumnNumber(bool *Invalid) const {
  assert(isValid());
  return SrcMgr->getSpellingColumnNumber(*this, Invalid);
}

bool FullSourceLoc::isInSystemHeader() const {
  assert(isValid());
  return SrcMgr->isInSystemHeader(*this);
}

bool FullSourceLoc::isBeforeInTranslationUnitThan(SourceLocation Loc) const {
  assert(isValid());
  return SrcMgr->isBeforeInTranslationUnit(*this, Loc);
}

void FullSourceLoc::dump() const {
  SourceLocation::dump(*SrcMgr);
}

const char *FullSourceLoc::getCharacterData(bool *Invalid) const {
  assert(isValid());
  return SrcMgr->getCharacterData(*this, Invalid);
}

const llvm::MemoryBuffer* FullSourceLoc::getBuffer(bool *Invalid) const {
  assert(isValid());
  return SrcMgr->getBuffer(SrcMgr->getFileID(*this), Invalid);
}

StringRef FullSourceLoc::getBufferData(bool *Invalid) const {
  return getBuffer(Invalid)->getBuffer();
}

std::pair<FileID, unsigned> FullSourceLoc::getDecomposedLoc() const {
  return SrcMgr->getDecomposedLoc(*this);
}