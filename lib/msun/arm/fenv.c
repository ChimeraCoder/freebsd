
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

#define	__fenv_static
#include "fenv.h"

/*
 * The following macros map between the softfloat emulator's flags and
 * the hardware's FPSR.  The hardware this file was written for doesn't
 * have rounding control bits, so we stick those in the system ID byte.
 */
#define	__set_env(env, flags, mask, rnd) env = ((flags)			\
						| (mask)<<_FPUSW_SHIFT	\
						| (rnd) << 24)
#define	__env_flags(env)		((env) & FE_ALL_EXCEPT)
#define	__env_mask(env)			(((env) >> _FPUSW_SHIFT)	\
						& FE_ALL_EXCEPT)
#define	__env_round(env)		(((env) >> 24) & _ROUND_MASK)
#include "fenv-softfloat.h"

#ifdef __GNUC_GNU_INLINE__
#error "This file must be compiled with C99 'inline' semantics"
#endif

/*
 * Hopefully the system ID byte is immutable, so it's valid to use
 * this as a default environment.
 */
const fenv_t __fe_dfl_env = 0;

extern inline int feclearexcept(int __excepts);
extern inline int fegetexceptflag(fexcept_t *__flagp, int __excepts);
extern inline int fesetexceptflag(const fexcept_t *__flagp, int __excepts);
extern inline int feraiseexcept(int __excepts);
extern inline int fetestexcept(int __excepts);
extern inline int fegetround(void);
extern inline int fesetround(int __round);
extern inline int fegetenv(fenv_t *__envp);
extern inline int feholdexcept(fenv_t *__envp);
extern inline int fesetenv(const fenv_t *__envp);
extern inline int feupdateenv(const fenv_t *__envp);