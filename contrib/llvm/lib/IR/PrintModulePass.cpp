
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

#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {

  class PrintModulePass : public ModulePass {
    std::string Banner;
    raw_ostream *Out;       // raw_ostream to print on
    bool DeleteStream;      // Delete the ostream in our dtor?
  public:
    static char ID;
    PrintModulePass() : ModulePass(ID), Out(&dbgs()), 
      DeleteStream(false) {}
    PrintModulePass(const std::string &B, raw_ostream *o, bool DS)
        : ModulePass(ID), Banner(B), Out(o), DeleteStream(DS) {}
    
    ~PrintModulePass() {
      if (DeleteStream) delete Out;
    }
    
    bool runOnModule(Module &M) {
      (*Out) << Banner << M;
      return false;
    }
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
  };
  
  class PrintFunctionPass : public FunctionPass {
    std::string Banner;     // String to print before each function
    raw_ostream *Out;       // raw_ostream to print on
    bool DeleteStream;      // Delete the ostream in our dtor?
  public:
    static char ID;
    PrintFunctionPass() : FunctionPass(ID), Banner(""), Out(&dbgs()), 
                          DeleteStream(false) {}
    PrintFunctionPass(const std::string &B, raw_ostream *o, bool DS)
      : FunctionPass(ID), Banner(B), Out(o), DeleteStream(DS) {}
    
    ~PrintFunctionPass() {
      if (DeleteStream) delete Out;
    }
    
    // runOnFunction - This pass just prints a banner followed by the
    // function as it's processed.
    //
    bool runOnFunction(Function &F) {
      (*Out) << Banner << static_cast<Value&>(F);
      return false;
    }
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
  };
  
  class PrintBasicBlockPass : public BasicBlockPass {
    std::string Banner;
    raw_ostream *Out;       // raw_ostream to print on
    bool DeleteStream;      // Delete the ostream in our dtor?
  public:
    static char ID;
    PrintBasicBlockPass() : BasicBlockPass(ID), Out(&dbgs()), 
      DeleteStream(false) {}
    PrintBasicBlockPass(const std::string &B, raw_ostream *o, bool DS)
        : BasicBlockPass(ID), Banner(B), Out(o), DeleteStream(DS) {}
    
    ~PrintBasicBlockPass() {
      if (DeleteStream) delete Out;
    }
    
    bool runOnBasicBlock(BasicBlock &BB) {
      (*Out) << Banner << BB;
      return false;
    }
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
  };
}

char PrintModulePass::ID = 0;
INITIALIZE_PASS(PrintModulePass, "print-module",
                "Print module to stderr", false, false)
char PrintFunctionPass::ID = 0;
INITIALIZE_PASS(PrintFunctionPass, "print-function",
                "Print function to stderr", false, false)
char PrintBasicBlockPass::ID = 0;
INITIALIZE_PASS(PrintBasicBlockPass, "print-bb",
                "Print BB to stderr", false, false)

/// createPrintModulePass - Create and return a pass that writes the
/// module to the specified raw_ostream.
ModulePass *llvm::createPrintModulePass(llvm::raw_ostream *OS, 
                                        bool DeleteStream,
                                        const std::string &Banner) {
  return new PrintModulePass(Banner, OS, DeleteStream);
}

/// createPrintFunctionPass - Create and return a pass that prints
/// functions to the specified raw_ostream as they are processed.
FunctionPass *llvm::createPrintFunctionPass(const std::string &Banner,
                                            llvm::raw_ostream *OS, 
                                            bool DeleteStream) {
  return new PrintFunctionPass(Banner, OS, DeleteStream);
}

/// createPrintBasicBlockPass - Create and return a pass that writes the
/// BB to the specified raw_ostream.
BasicBlockPass *llvm::createPrintBasicBlockPass(llvm::raw_ostream *OS,
                                        bool DeleteStream,
                                        const std::string &Banner) {
  return new PrintBasicBlockPass(Banner, OS, DeleteStream);
}