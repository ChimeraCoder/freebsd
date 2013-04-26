
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

#include <sys/types.h>
#include <sys/elf.h>
#include <sys/time.h>
#include <sys/vdso.h>
#include <machine/cpufunc.h>
#include "libc_private.h"

static u_int
__vdso_gettc_low(const struct vdso_timehands *th)
{
	uint32_t rv;

	__asm __volatile("rdtsc; shrd %%cl, %%edx, %0"
	    : "=a" (rv) : "c" (th->th_x86_shift) : "edx");
	return (rv);
}

#pragma weak __vdso_gettc
u_int
__vdso_gettc(const struct vdso_timehands *th)
{

	return (th->th_x86_shift > 0 ? __vdso_gettc_low(th) : rdtsc32());
}

#pragma weak __vdso_gettimekeep
int
__vdso_gettimekeep(struct vdso_timekeep **tk)
{

	return (_elf_aux_info(AT_TIMEKEEP, tk, sizeof(*tk)));
}