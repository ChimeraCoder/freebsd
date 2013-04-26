
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

#include <sys/types.h>

#define	__fenv_static
#include "fenv.h"

#ifdef __GNUC_GNU_INLINE__
#error "This file must be compiled with C99 'inline' semantics"
#endif

const fenv_t __fe_dfl_env = 0x0009804c8a70033fULL;

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

/*
 * It doesn't pay to inline feupdateenv() because it includes one of
 * the rare uses of feraiseexcept() where the argument is not a
 * constant.  Thus, no dead code elimination can occur, resulting in
 * significant bloat.
 */
int
feupdateenv(const fenv_t *envp)
{
	fenv_t fpsr;

	__stfpsr(&fpsr);
	__ldfpsr(*envp);
	feraiseexcept((fpsr >> _FPUSW_SHIFT) & FE_ALL_EXCEPT);
	return (0);
}