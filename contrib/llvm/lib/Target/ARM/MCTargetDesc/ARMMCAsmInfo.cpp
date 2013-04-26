
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

#include "ARMMCAsmInfo.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

cl::opt<bool>
EnableARMEHABI("arm-enable-ehabi", cl::Hidden,
  cl::desc("Generate ARM EHABI tables"),
  cl::init(false));


void ARMMCAsmInfoDarwin::anchor() { }

ARMMCAsmInfoDarwin::ARMMCAsmInfoDarwin() {
  Data64bitsDirective = 0;
  CommentString = "@";
  Code16Directive = ".code\t16";
  Code32Directive = ".code\t32";
  UseDataRegionDirectives = true;

  SupportsDebugInformation = true;

  // Exceptions handling
  ExceptionsType = ExceptionHandling::SjLj;
}

void ARMELFMCAsmInfo::anchor() { }

ARMELFMCAsmInfo::ARMELFMCAsmInfo() {
  // ".comm align is in bytes but .align is pow-2."
  AlignmentIsInBytes = false;

  Data64bitsDirective = 0;
  CommentString = "@";
  PrivateGlobalPrefix = ".L";
  Code16Directive = ".code\t16";
  Code32Directive = ".code\t32";

  WeakRefDirective = "\t.weak\t";

  HasLEB128 = true;
  SupportsDebugInformation = true;

  // Exceptions handling
  if (EnableARMEHABI)
    ExceptionsType = ExceptionHandling::ARM;
}