
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

#include "llvm/Support/CommandLine.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

using namespace clang::tooling;
using namespace llvm;

const char *const CommonOptionsParser::HelpMessage =
    "\n"
    "-p <build-path> is used to read a compile command database.\n"
    "\n"
    "\tFor example, it can be a CMake build directory in which a file named\n"
    "\tcompile_commands.json exists (use -DCMAKE_EXPORT_COMPILE_COMMANDS=ON\n"
    "\tCMake option to get this output). When no build path is specified,\n"
    "\ta search for compile_commands.json will be attempted through all\n"
    "\tparent paths of the first input file . See:\n"
    "\thttp://clang.llvm.org/docs/HowToSetupToolingForLLVM.html for an\n"
    "\texample of setting up Clang Tooling on a source tree.\n"
    "\n"
    "<source0> ... specify the paths of source files. These paths are\n"
    "\tlooked up in the compile command database. If the path of a file is\n"
    "\tabsolute, it needs to point into CMake's source tree. If the path is\n"
    "\trelative, the current working directory needs to be in the CMake\n"
    "\tsource tree and the file must be in a subdirectory of the current\n"
    "\tworking directory. \"./\" prefixes in the relative files will be\n"
    "\tautomatically removed, but the rest of a relative path must be a\n"
    "\tsuffix of a path in the compile command database.\n"
    "\n";

CommonOptionsParser::CommonOptionsParser(int &argc, const char **argv) {
  static cl::opt<std::string> BuildPath(
      "p", cl::desc("Build path"), cl::Optional);

  static cl::list<std::string> SourcePaths(
      cl::Positional, cl::desc("<source0> [... <sourceN>]"), cl::OneOrMore);

  Compilations.reset(FixedCompilationDatabase::loadFromCommandLine(argc,
                                                                   argv));
  cl::ParseCommandLineOptions(argc, argv);
  SourcePathList = SourcePaths;
  if (!Compilations) {
    std::string ErrorMessage;
    if (!BuildPath.empty()) {
      Compilations.reset(CompilationDatabase::autoDetectFromDirectory(
                              BuildPath, ErrorMessage));
    } else {
      Compilations.reset(CompilationDatabase::autoDetectFromSource(
                              SourcePaths[0], ErrorMessage));
    }
    if (!Compilations)
      llvm::report_fatal_error(ErrorMessage);
  }
}