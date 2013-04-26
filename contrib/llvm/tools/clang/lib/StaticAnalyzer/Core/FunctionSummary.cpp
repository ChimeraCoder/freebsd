
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

#include "clang/StaticAnalyzer/Core/PathSensitive/FunctionSummary.h"
using namespace clang;
using namespace ento;

unsigned FunctionSummariesTy::getTotalNumBasicBlocks() {
  unsigned Total = 0;
  for (MapTy::iterator I = Map.begin(), E = Map.end(); I != E; ++I) {
    Total += I->second.TotalBasicBlocks;
  }
  return Total;
}

unsigned FunctionSummariesTy::getTotalNumVisitedBasicBlocks() {
  unsigned Total = 0;
  for (MapTy::iterator I = Map.begin(), E = Map.end(); I != E; ++I) {
    Total += I->second.VisitedBasicBlocks.count();
  }
  return Total;
}