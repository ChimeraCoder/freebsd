
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

#include "llvm/CodeGen/GCs.h"
#include "llvm/CodeGen/GCStrategy.h"

using namespace llvm;

namespace {
  class OcamlGC : public GCStrategy {
  public:
    OcamlGC();
  };
}

static GCRegistry::Add<OcamlGC>
X("ocaml", "ocaml 3.10-compatible GC");

void llvm::linkOcamlGC() { }

OcamlGC::OcamlGC() {
  NeededSafePoints = 1 << GC::PostCall;
  UsesMetadata = true;
}