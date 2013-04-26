
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

#include "llvm/Linker.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"
using namespace llvm;

Linker::Linker(StringRef progname, StringRef modname,
               LLVMContext& C, unsigned flags):
  Context(C),
  Composite(new Module(modname, C)),
  Flags(flags),
  Error(),
  ProgramName(progname) { }

Linker::Linker(StringRef progname, Module* aModule, unsigned flags) :
  Context(aModule->getContext()),
  Composite(aModule),
  Flags(flags),
  Error(),
  ProgramName(progname) { }

Linker::~Linker() {
  delete Composite;
}

bool
Linker::error(StringRef message) {
  Error = message;
  if (!(Flags&QuietErrors))
    errs() << ProgramName << ": error: " << message << "\n";
  return true;
}

bool
Linker::warning(StringRef message) {
  Error = message;
  if (!(Flags&QuietWarnings))
    errs() << ProgramName << ": warning: " << message << "\n";
  return false;
}

void
Linker::verbose(StringRef message) {
  if (Flags&Verbose)
    errs() << "  " << message << "\n";
}

Module*
Linker::releaseModule() {
  Module* result = Composite;
  Error.clear();
  Composite = 0;
  Flags = 0;
  return result;
}