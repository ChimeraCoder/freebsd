
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

#include "clang/Frontend/FrontendOptions.h"
#include "llvm/ADT/StringSwitch.h"
using namespace clang;

InputKind FrontendOptions::getInputKindForExtension(StringRef Extension) {
  return llvm::StringSwitch<InputKind>(Extension)
    .Cases("ast", "pcm", IK_AST)
    .Case("c", IK_C)
    .Cases("S", "s", IK_Asm)
    .Case("i", IK_PreprocessedC)
    .Case("ii", IK_PreprocessedCXX)
    .Case("m", IK_ObjC)
    .Case("mi", IK_PreprocessedObjC)
    .Cases("mm", "M", IK_ObjCXX)
    .Case("mii", IK_PreprocessedObjCXX)
    .Case("C", IK_CXX)
    .Cases("C", "cc", "cp", IK_CXX)
    .Cases("cpp", "CPP", "c++", "cxx", "hpp", IK_CXX)
    .Case("cl", IK_OpenCL)
    .Case("cu", IK_CUDA)
    .Cases("ll", "bc", IK_LLVM_IR)
    .Default(IK_C);
}