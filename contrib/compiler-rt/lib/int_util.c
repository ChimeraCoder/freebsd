
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

#include "int_util.h"
#include "int_lib.h"

/* NOTE: The definitions in this file are declared weak because we clients to be
 * able to arbitrarily package individual functions into separate .a files. If
 * we did not declare these weak, some link situations might end up seeing
 * duplicate strong definitions of the same symbol.
 *
 * We can't use this solution for kernel use (which may not support weak), but
 * currently expect that when built for kernel use all the functionality is
 * packaged into a single library.
 */

#ifdef KERNEL_USE

extern void panic(const char *, ...) __attribute__((noreturn));
__attribute__((visibility("hidden")))
void compilerrt_abort_impl(const char *file, int line, const char *function) {
  panic("%s:%d: abort in %s", file, line, function);
}

#elif __APPLE__ && !__STATIC__

/* from libSystem.dylib */
extern void __assert_rtn(const char *func, const char *file, 
                     int line, const char * message) __attribute__((noreturn));

__attribute__((weak))
__attribute__((visibility("hidden")))
void compilerrt_abort_impl(const char *file, int line, const char *function) {
  __assert_rtn(function, file, line, "libcompiler_rt abort");
}


#else

/* Get the system definition of abort() */
#include <stdlib.h>

__attribute__((weak))
__attribute__((visibility("hidden")))
void compilerrt_abort_impl(const char *file, int line, const char *function) {
  abort();
}

#endif