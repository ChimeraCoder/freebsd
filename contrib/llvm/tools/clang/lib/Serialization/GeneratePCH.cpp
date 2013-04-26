
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

#include "clang/Serialization/ASTWriter.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/FileManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/SemaConsumer.h"
#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

using namespace clang;

PCHGenerator::PCHGenerator(const Preprocessor &PP,
                           StringRef OutputFile,
                           clang::Module *Module,
                           StringRef isysroot,
                           raw_ostream *OS)
  : PP(PP), OutputFile(OutputFile), Module(Module), 
    isysroot(isysroot.str()), Out(OS), 
    SemaPtr(0), Stream(Buffer), Writer(Stream) {
}

PCHGenerator::~PCHGenerator() {
}

void PCHGenerator::HandleTranslationUnit(ASTContext &Ctx) {
  if (PP.getDiagnostics().hasErrorOccurred())
    return;
  
  // Emit the PCH file
  assert(SemaPtr && "No Sema?");
  Writer.WriteAST(*SemaPtr, OutputFile, Module, isysroot);

  // Write the generated bitstream to "Out".
  Out->write((char *)&Buffer.front(), Buffer.size());

  // Make sure it hits disk now.
  Out->flush();

  // Free up some memory, in case the process is kept alive.
  Buffer.clear();
}

ASTMutationListener *PCHGenerator::GetASTMutationListener() {
  return &Writer;
}

ASTDeserializationListener *PCHGenerator::GetASTDeserializationListener() {
  return &Writer;
}