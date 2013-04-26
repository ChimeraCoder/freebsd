
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

#include "clang/Basic/Version.h"
#include "clang/Basic/LLVM.h"
#include "llvm/Config/config.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <cstring>

#ifdef HAVE_SVN_VERSION_INC
#  include "SVNVersion.inc"
#endif

namespace clang {

std::string getClangRepositoryPath() {
#if defined(CLANG_REPOSITORY_STRING)
  return CLANG_REPOSITORY_STRING;
#else
#ifdef SVN_REPOSITORY
  StringRef URL(SVN_REPOSITORY);
#else
  StringRef URL("");
#endif

  // If the SVN_REPOSITORY is empty, try to use the SVN keyword. This helps us
  // pick up a tag in an SVN export, for example.
  static StringRef SVNRepository("$URL: http://llvm.org/svn/llvm-project/cfe/trunk/lib/Basic/Version.cpp $");
  if (URL.empty()) {
    URL = SVNRepository.slice(SVNRepository.find(':'),
                              SVNRepository.find("/lib/Basic"));
  }

  // Strip off version from a build from an integration branch.
  URL = URL.slice(0, URL.find("/src/tools/clang"));

  // Trim path prefix off, assuming path came from standard cfe path.
  size_t Start = URL.find("cfe/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start + 4);

  return URL;
#endif
}

std::string getLLVMRepositoryPath() {
#ifdef LLVM_REPOSITORY
  StringRef URL(LLVM_REPOSITORY);
#else
  StringRef URL("");
#endif

  // Trim path prefix off, assuming path came from standard llvm path.
  // Leave "llvm/" prefix to distinguish the following llvm revision from the
  // clang revision.
  size_t Start = URL.find("llvm/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start);

  return URL;
}

std::string getClangRevision() {
#ifdef SVN_REVISION
  return SVN_REVISION;
#else
  return "";
#endif
}

std::string getLLVMRevision() {
#ifdef LLVM_REVISION
  return LLVM_REVISION;
#else
  return "";
#endif
}

std::string getClangFullRepositoryVersion() {
  std::string buf;
  llvm::raw_string_ostream OS(buf);
  std::string Path = getClangRepositoryPath();
  std::string Revision = getClangRevision();
  if (!Path.empty() || !Revision.empty()) {
    OS << '(';
    if (!Path.empty())
      OS << Path;
    if (!Revision.empty()) {
      if (!Path.empty())
        OS << ' ';
      OS << Revision;
    }
    OS << ')';
  }  
  // Support LLVM in a separate repository.
  std::string LLVMRev = getLLVMRevision();
  if (!LLVMRev.empty() && LLVMRev != Revision) {
    OS << " (";    
    std::string LLVMRepo = getLLVMRepositoryPath();
    if (!LLVMRepo.empty())
      OS << LLVMRepo << ' ';
    OS << LLVMRev << ')';
  }
  return OS.str();
}

std::string getClangFullVersion() {
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef CLANG_VENDOR
  OS << CLANG_VENDOR;
#endif
  OS << "clang version " CLANG_VERSION_STRING " "
     << getClangFullRepositoryVersion();

#ifdef CLANG_VENDOR_SUFFIX
  OS << CLANG_VENDOR_SUFFIX;
#elif defined(CLANG_VENDOR)
  // If vendor supplied, include the base LLVM version as well.
  OS << " (based on LLVM " << PACKAGE_VERSION << ")";
#endif

  return OS.str();
}

std::string getClangFullCPPVersion() {
  // The version string we report in __VERSION__ is just a compacted version of
  // the one we report on the command line.
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef CLANG_VENDOR
  OS << CLANG_VENDOR;
#endif
  OS << "Clang " CLANG_VERSION_STRING " " << getClangFullRepositoryVersion();
  return OS.str();
}

} // end namespace clang