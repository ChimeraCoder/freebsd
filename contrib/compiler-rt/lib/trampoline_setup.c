
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

extern void __clear_cache(void* start, void* end);

/*
 * The ppc compiler generates calls to __trampoline_setup() when creating 
 * trampoline functions on the stack for use with nested functions.
 * This function creates a custom 40-byte trampoline function on the stack 
 * which loads r11 with a pointer to the outer function's locals
 * and then jumps to the target nested function.
 */

#if __ppc__ && !defined(__powerpc64__)
void __trampoline_setup(uint32_t* trampOnStack, int trampSizeAllocated, 
                                const void* realFunc, void* localsPtr)
{
    /* should never happen, but if compiler did not allocate */
    /* enough space on stack for the trampoline, abort */
    if ( trampSizeAllocated < 40 )
        compilerrt_abort();
    
    /* create trampoline */
    trampOnStack[0] = 0x7c0802a6;    /* mflr r0 */
    trampOnStack[1] = 0x4800000d;    /* bl Lbase */
    trampOnStack[2] = (uint32_t)realFunc;
    trampOnStack[3] = (uint32_t)localsPtr;
    trampOnStack[4] = 0x7d6802a6;    /* Lbase: mflr r11 */
    trampOnStack[5] = 0x818b0000;    /* lwz    r12,0(r11) */
    trampOnStack[6] = 0x7c0803a6;    /* mtlr r0 */
    trampOnStack[7] = 0x7d8903a6;    /* mtctr r12 */
    trampOnStack[8] = 0x816b0004;    /* lwz    r11,4(r11) */
    trampOnStack[9] = 0x4e800420;    /* bctr */
    
    /* clear instruction cache */
    __clear_cache(trampOnStack, &trampOnStack[10]);
}
#endif /* __ppc__ && !defined(__powerpc64__) */