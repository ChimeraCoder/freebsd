
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

#include "Interpreter.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include <cstring>
using namespace llvm;

namespace {

static struct RegisterInterp {
  RegisterInterp() { Interpreter::Register(); }
} InterpRegistrator;

}

extern "C" void LLVMLinkInInterpreter() { }

/// create - Create a new interpreter object.  This can never fail.
///
ExecutionEngine *Interpreter::create(Module *M, std::string* ErrStr) {
  // Tell this Module to materialize everything and release the GVMaterializer.
  if (M->MaterializeAllPermanently(ErrStr))
    // We got an error, just return 0
    return 0;

  return new Interpreter(M);
}

//===----------------------------------------------------------------------===//
// Interpreter ctor - Initialize stuff
//
Interpreter::Interpreter(Module *M)
  : ExecutionEngine(M), TD(M) {
      
  memset(&ExitValue.Untyped, 0, sizeof(ExitValue.Untyped));
  setDataLayout(&TD);
  // Initialize the "backend"
  initializeExecutionEngine();
  initializeExternalFunctions();
  emitGlobals();

  IL = new IntrinsicLowering(TD);
}

Interpreter::~Interpreter() {
  delete IL;
}

void Interpreter::runAtExitHandlers () {
  while (!AtExitHandlers.empty()) {
    callFunction(AtExitHandlers.back(), std::vector<GenericValue>());
    AtExitHandlers.pop_back();
    run();
  }
}

/// run - Start execution with the specified function and arguments.
///
GenericValue
Interpreter::runFunction(Function *F,
                         const std::vector<GenericValue> &ArgValues) {
  assert (F && "Function *F was null at entry to run()");

  // Try extra hard not to pass extra args to a function that isn't
  // expecting them.  C programmers frequently bend the rules and
  // declare main() with fewer parameters than it actually gets
  // passed, and the interpreter barfs if you pass a function more
  // parameters than it is declared to take. This does not attempt to
  // take into account gratuitous differences in declared types,
  // though.
  std::vector<GenericValue> ActualArgs;
  const unsigned ArgCount = F->getFunctionType()->getNumParams();
  for (unsigned i = 0; i < ArgCount; ++i)
    ActualArgs.push_back(ArgValues[i]);

  // Set up the function call.
  callFunction(F, ActualArgs);

  // Start executing the function.
  run();

  return ExitValue;
}