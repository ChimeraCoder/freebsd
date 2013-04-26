
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

#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/circular_raw_ostream.h"

using namespace llvm;

// All Debug.h functionality is a no-op in NDEBUG mode.
#ifndef NDEBUG
bool llvm::DebugFlag;  // DebugFlag - Exported boolean set by the -debug option

// -debug - Command line option to enable the DEBUG statements in the passes.
// This flag may only be enabled in debug builds.
static cl::opt<bool, true>
Debug("debug", cl::desc("Enable debug output"), cl::Hidden,
      cl::location(DebugFlag));

// -debug-buffer-size - Buffer the last N characters of debug output
//until program termination.
static cl::opt<unsigned>
DebugBufferSize("debug-buffer-size",
                cl::desc("Buffer the last N characters of debug output "
                         "until program termination. "
                         "[default 0 -- immediate print-out]"),
                cl::Hidden,
                cl::init(0));

static std::string CurrentDebugType;

namespace {

struct DebugOnlyOpt {
  void operator=(const std::string &Val) const {
    DebugFlag |= !Val.empty();
    CurrentDebugType = Val;
  }
};

}

static DebugOnlyOpt DebugOnlyOptLoc;

static cl::opt<DebugOnlyOpt, true, cl::parser<std::string> >
DebugOnly("debug-only", cl::desc("Enable a specific type of debug output"),
          cl::Hidden, cl::value_desc("debug string"),
          cl::location(DebugOnlyOptLoc), cl::ValueRequired);

// Signal handlers - dump debug output on termination.
static void debug_user_sig_handler(void *Cookie) {
  // This is a bit sneaky.  Since this is under #ifndef NDEBUG, we
  // know that debug mode is enabled and dbgs() really is a
  // circular_raw_ostream.  If NDEBUG is defined, then dbgs() ==
  // errs() but this will never be invoked.
  llvm::circular_raw_ostream *dbgout =
    static_cast<llvm::circular_raw_ostream *>(&llvm::dbgs());
  dbgout->flushBufferWithBanner();
}

// isCurrentDebugType - Return true if the specified string is the debug type
// specified on the command line, or if none was specified on the command line
// with the -debug-only=X option.
//
bool llvm::isCurrentDebugType(const char *DebugType) {
  return CurrentDebugType.empty() || DebugType == CurrentDebugType;
}

/// setCurrentDebugType - Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
void llvm::setCurrentDebugType(const char *Type) {
  CurrentDebugType = Type;
}

/// dbgs - Return a circular-buffered debug stream.
raw_ostream &llvm::dbgs() {
  // Do one-time initialization in a thread-safe way.
  static struct dbgstream {
    circular_raw_ostream strm;

    dbgstream() :
        strm(errs(), "*** Debug Log Output ***\n",
             (!EnableDebugBuffering || !DebugFlag) ? 0 : DebugBufferSize) {
      if (EnableDebugBuffering && DebugFlag && DebugBufferSize != 0)
        // TODO: Add a handler for SIGUSER1-type signals so the user can
        // force a debug dump.
        sys::AddSignalHandler(&debug_user_sig_handler, 0);
      // Otherwise we've already set the debug stream buffer size to
      // zero, disabling buffering so it will output directly to errs().
    }
  } thestrm;

  return thestrm.strm;
}

#else
// Avoid "has no symbols" warning.
namespace llvm {
  /// dbgs - Return errs().
  raw_ostream &dbgs() {
    return errs();
  }
}

#endif

/// EnableDebugBuffering - Turn on signal handler installation.
///
bool llvm::EnableDebugBuffering = false;