
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

#include "llvm/CodeGen/GCMetadataPrinter.h"
using namespace llvm;

GCMetadataPrinter::GCMetadataPrinter() { }

GCMetadataPrinter::~GCMetadataPrinter() { }

void GCMetadataPrinter::beginAssembly(AsmPrinter &AP) {
  // Default is no action.
}

void GCMetadataPrinter::finishAssembly(AsmPrinter &AP) {
  // Default is no action.
}