
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

#include <string.h>

// Must declare the symbols in the global namespace.
static void *DoSearch(const char* symbolName) {
#define EXPLICIT_SYMBOL(SYM) \
   extern void *SYM; if (!strcmp(symbolName, #SYM)) return &SYM

  // If this is darwin, it has some funky issues, try to solve them here.  Some
  // important symbols are marked 'private external' which doesn't allow
  // SearchForAddressOfSymbol to find them.  As such, we special case them here,
  // there is only a small handful of them.

#ifdef __APPLE__
  {
    // __eprintf is sometimes used for assert() handling on x86.
    //
    // FIXME: Currently disabled when using Clang, as we don't always have our
    // runtime support libraries available.
#ifndef __clang__
#ifdef __i386__
    EXPLICIT_SYMBOL(__eprintf);
#endif
#endif
  }
#endif

#ifdef __CYGWIN__
  {
    EXPLICIT_SYMBOL(_alloca);
    EXPLICIT_SYMBOL(__main);
  }
#endif

#undef EXPLICIT_SYMBOL
  return 0;
}

namespace llvm {
void *SearchForAddressOfSpecialSymbol(const char* symbolName) {
  return DoSearch(symbolName);
}
}  // namespace llvm