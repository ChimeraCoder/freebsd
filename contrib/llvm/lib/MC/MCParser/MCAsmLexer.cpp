
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

#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/Support/SourceMgr.h"

using namespace llvm;

MCAsmLexer::MCAsmLexer() : CurTok(AsmToken::Error, StringRef()),
                           TokStart(0), SkipSpace(true) {
}

MCAsmLexer::~MCAsmLexer() {
}

SMLoc MCAsmLexer::getLoc() const {
  return SMLoc::getFromPointer(TokStart);
}

SMLoc AsmToken::getLoc() const {
  return SMLoc::getFromPointer(Str.data());
}

SMLoc AsmToken::getEndLoc() const {
  return SMLoc::getFromPointer(Str.data() + Str.size());
}