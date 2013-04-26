
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

#include "clang/Basic/TokenKinds.h"
#include <cassert>
using namespace clang;

static const char * const TokNames[] = {
#define TOK(X) #X,
#define KEYWORD(X,Y) #X,
#include "clang/Basic/TokenKinds.def"
  0
};

const char *tok::getTokenName(enum TokenKind Kind) {
  assert(Kind < tok::NUM_TOKENS);
  return TokNames[Kind];
}

const char *tok::getTokenSimpleSpelling(enum TokenKind Kind) {
  switch (Kind) {
#define PUNCTUATOR(X,Y) case X: return Y;
#include "clang/Basic/TokenKinds.def"
  default: break;
  }

  return 0;
}