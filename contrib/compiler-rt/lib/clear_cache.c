
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

#include "int_lib.h"

#if __APPLE__
  #include <libkern/OSCacheControl.h>
#endif

/*
 * The compiler generates calls to __clear_cache() when creating 
 * trampoline functions on the stack for use with nested functions.
 * It is expected to invalidate the instruction cache for the 
 * specified range.
 */

void __clear_cache(void* start, void* end)
{
#if __i386__ || __x86_64__
/*
 * Intel processors have a unified instruction and data cache
 * so there is nothing to do
 */
#else
    #if __APPLE__
        /* On Darwin, sys_icache_invalidate() provides this functionality */
        sys_icache_invalidate(start, end-start);
    #else
        compilerrt_abort();
    #endif
#endif
}