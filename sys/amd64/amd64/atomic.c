
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/* This file creates publically callable functions to perform various
 * simple arithmetic on memory which is atomic in the presence of
 * interrupts and multiple processors.
 */
#include <sys/types.h>

/* Firstly make atomic.h generate prototypes as it will for kernel modules */
#define KLD_MODULE
#include <machine/atomic.h>
#undef _MACHINE_ATOMIC_H_	/* forget we included it */
#undef KLD_MODULE
#undef ATOMIC_ASM

/* Make atomic.h generate public functions */
#define WANT_FUNCTIONS
#define static
#undef __inline
#define __inline

#include <machine/atomic.h>