
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

#include "clang/StaticAnalyzer/Checkers/ClangCheckers.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"

// FIXME: This is only necessary as long as there are checker registration
// functions that do additional work besides mgr.registerChecker<CLASS>().
// The only checkers that currently do this are:
// - NSAutoreleasePoolChecker
// - NSErrorChecker
// - ObjCAtSyncChecker
// It's probably worth including this information in Checkers.td to minimize
// boilerplate code.
#include "ClangSACheckers.h"

using namespace clang;
using namespace ento;

void ento::registerBuiltinCheckers(CheckerRegistry &registry) {
#define GET_CHECKERS
#define CHECKER(FULLNAME,CLASS,DESCFILE,HELPTEXT,GROUPINDEX,HIDDEN)    \
  registry.addChecker(register##CLASS, FULLNAME, HELPTEXT);
#include "Checkers.inc"
#undef GET_CHECKERS
}