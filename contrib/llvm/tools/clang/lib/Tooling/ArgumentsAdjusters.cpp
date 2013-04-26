
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

#include "clang/Tooling/ArgumentsAdjusters.h"

namespace clang {
namespace tooling {

void ArgumentsAdjuster::anchor() {
}

/// Add -fsyntax-only option to the commnand line arguments.
CommandLineArguments
ClangSyntaxOnlyAdjuster::Adjust(const CommandLineArguments &Args) {
  CommandLineArguments AdjustedArgs = Args;
  // FIXME: Remove options that generate output.
  AdjustedArgs.push_back("-fsyntax-only");
  return AdjustedArgs;
}

} // end namespace tooling
} // end namespace clang