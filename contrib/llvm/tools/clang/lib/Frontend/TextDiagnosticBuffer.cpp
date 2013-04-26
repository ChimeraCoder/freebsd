
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

#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ErrorHandling.h"
using namespace clang;

/// HandleDiagnostic - Store the errors, warnings, and notes that are
/// reported.
///
void TextDiagnosticBuffer::HandleDiagnostic(DiagnosticsEngine::Level Level,
                                            const Diagnostic &Info) {
  // Default implementation (Warnings/errors count).
  DiagnosticConsumer::HandleDiagnostic(Level, Info);

  SmallString<100> Buf;
  Info.FormatDiagnostic(Buf);
  switch (Level) {
  default: llvm_unreachable(
                         "Diagnostic not handled during diagnostic buffering!");
  case DiagnosticsEngine::Note:
    Notes.push_back(std::make_pair(Info.getLocation(), Buf.str()));
    break;
  case DiagnosticsEngine::Warning:
    Warnings.push_back(std::make_pair(Info.getLocation(), Buf.str()));
    break;
  case DiagnosticsEngine::Error:
  case DiagnosticsEngine::Fatal:
    Errors.push_back(std::make_pair(Info.getLocation(), Buf.str()));
    break;
  }
}

/// \brief Escape diagnostic texts to avoid problems when they are fed into the
/// diagnostic formatter a second time.
static StringRef escapeDiag(StringRef Str, SmallVectorImpl<char> &Buf) {
  size_t Pos = Str.find('%');
  if (Pos == StringRef::npos)
    return Str;

  // We found a '%'. Replace this and all following '%' with '%%'.
  Buf.clear();
  Buf.append(Str.data(), Str.data() + Pos);
  for (size_t I = Pos, E = Str.size(); I != E; ++I) {
    if (Str[I] == '%')
      Buf.push_back('%');
    Buf.push_back(Str[I]);
  }

  return StringRef(Buf.data(), Buf.size());
}

void TextDiagnosticBuffer::FlushDiagnostics(DiagnosticsEngine &Diags) const {
  SmallVector<char, 64> Buf;
  // FIXME: Flush the diagnostics in order.
  for (const_iterator it = err_begin(), ie = err_end(); it != ie; ++it)
    Diags.Report(Diags.getCustomDiagID(DiagnosticsEngine::Error,
                                       escapeDiag(it->second, Buf)));
  for (const_iterator it = warn_begin(), ie = warn_end(); it != ie; ++it)
    Diags.Report(Diags.getCustomDiagID(DiagnosticsEngine::Warning,
                                       escapeDiag(it->second, Buf)));
  for (const_iterator it = note_begin(), ie = note_end(); it != ie; ++it)
    Diags.Report(Diags.getCustomDiagID(DiagnosticsEngine::Note,
                                       escapeDiag(it->second, Buf)));
}

DiagnosticConsumer *TextDiagnosticBuffer::clone(DiagnosticsEngine &) const {
  return new TextDiagnosticBuffer();
}