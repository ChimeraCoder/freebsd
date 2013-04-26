
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

#include "XCoreMCAsmInfo.h"
#include "llvm/ADT/StringRef.h"
using namespace llvm;

void XCoreMCAsmInfo::anchor() { }

XCoreMCAsmInfo::XCoreMCAsmInfo(const Target &T, StringRef TT) {
  SupportsDebugInformation = true;
  Data16bitsDirective = "\t.short\t";
  Data32bitsDirective = "\t.long\t";
  Data64bitsDirective = 0;
  ZeroDirective = "\t.space\t";
  CommentString = "#";
    
  PrivateGlobalPrefix = ".L";
  AscizDirective = ".asciiz";
  WeakDefDirective = "\t.weak\t";
  WeakRefDirective = "\t.weak\t";

  // Debug
  HasLEB128 = true;
}