
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
#include <sys/types.h>
#include <machine/npx.h>

#define	__fenv_static
#include "fenv.h"

#ifdef __GNUC_GNU_INLINE__
#error "This file must be compiled with C99 'inline' semantics"
#endif

const fenv_t __fe_dfl_env = {
	__INITIAL_NPXCW__,
	0x0000,
	0x0000,
	0x1f80,
	0xffffffff,
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff }
};

enum __sse_support __has_sse =
#ifdef __SSE__
	__SSE_YES;
#else
	__SSE_UNK;
#endif

#define	getfl(x)	__asm __volatile("pushfl\n\tpopl %0" : "=mr" (*(x)))
#define	setfl(x)	__asm __volatile("pushl %0\n\tpopfl" : : "g" (x))
#define	cpuid_dx(x)	__asm __volatile("pushl %%ebx\n\tmovl $1, %%eax\n\t"  \
					 "cpuid\n\tpopl %%ebx"		      \
					: "=d" (*(x)) : : "eax", "ecx")

/*
 * Test for SSE support on this processor.  We need to do this because
 * we need to use ldmxcsr/stmxcsr to get correct results if any part
 * of the program was compiled to use SSE floating-point, but we can't
 * use SSE on older processors.
 */
int
__test_sse(void)
{
	int flag, nflag;
	int dx_features;

	/* Am I a 486? */
	getfl(&flag);
	nflag = flag ^ 0x200000;
	setfl(nflag);
	getfl(&nflag);
	if (flag != nflag) {
		/* Not a 486, so CPUID should work. */
		cpuid_dx(&dx_features);
		if (dx_features & 0x2000000) {
			__has_sse = __SSE_YES;
			return (1);
		}
	}
	__has_sse = __SSE_NO;
	return (0);
}

extern inline int feclearexcept(int __excepts);
extern inline int fegetexceptflag(fexcept_t *__flagp, int __excepts);

int
fesetexceptflag(const fexcept_t *flagp, int excepts)
{
	fenv_t env;
	__uint32_t mxcsr;

	__fnstenv(&env);
	env.__status &= ~excepts;
	env.__status |= *flagp & excepts;
	__fldenv(env);

	if (__HAS_SSE()) {
		__stmxcsr(&mxcsr);
		mxcsr &= ~excepts;
		mxcsr |= *flagp & excepts;
		__ldmxcsr(mxcsr);
	}

	return (0);
}

int
feraiseexcept(int excepts)
{
	fexcept_t ex = excepts;

	fesetexceptflag(&ex, excepts);
	__fwait();
	return (0);
}

extern inline int fetestexcept(int __excepts);
extern inline int fegetround(void);
extern inline int fesetround(int __round);

int
fegetenv(fenv_t *envp)
{
	__uint32_t mxcsr;

	__fnstenv(envp);
	/*
	 * fnstenv masks all exceptions, so we need to restore
	 * the old control word to avoid this side effect.
	 */
	__fldcw(envp->__control);
	if (__HAS_SSE()) {
		__stmxcsr(&mxcsr);
		__set_mxcsr(*envp, mxcsr);
	}
	return (0);
}

int
feholdexcept(fenv_t *envp)
{
	__uint32_t mxcsr;

	__fnstenv(envp);
	__fnclex();
	if (__HAS_SSE()) {
		__stmxcsr(&mxcsr);
		__set_mxcsr(*envp, mxcsr);
		mxcsr &= ~FE_ALL_EXCEPT;
		mxcsr |= FE_ALL_EXCEPT << _SSE_EMASK_SHIFT;
		__ldmxcsr(mxcsr);
	}
	return (0);
}

extern inline int fesetenv(const fenv_t *__envp);

int
feupdateenv(const fenv_t *envp)
{
	__uint32_t mxcsr;
	__uint16_t status;

	__fnstsw(&status);
	if (__HAS_SSE())
		__stmxcsr(&mxcsr);
	else
		mxcsr = 0;
	fesetenv(envp);
	feraiseexcept((mxcsr | status) & FE_ALL_EXCEPT);
	return (0);
}

int
__feenableexcept(int mask)
{
	__uint32_t mxcsr, omask;
	__uint16_t control;

	mask &= FE_ALL_EXCEPT;
	__fnstcw(&control);
	if (__HAS_SSE())
		__stmxcsr(&mxcsr);
	else
		mxcsr = 0;
	omask = ~(control | mxcsr >> _SSE_EMASK_SHIFT) & FE_ALL_EXCEPT;
	control &= ~mask;
	__fldcw(control);
	if (__HAS_SSE()) {
		mxcsr &= ~(mask << _SSE_EMASK_SHIFT);
		__ldmxcsr(mxcsr);
	}
	return (omask);
}

int
__fedisableexcept(int mask)
{
	__uint32_t mxcsr, omask;
	__uint16_t control;

	mask &= FE_ALL_EXCEPT;
	__fnstcw(&control);
	if (__HAS_SSE())
		__stmxcsr(&mxcsr);
	else
		mxcsr = 0;
	omask = ~(control | mxcsr >> _SSE_EMASK_SHIFT) & FE_ALL_EXCEPT;
	control |= mask;
	__fldcw(control);
	if (__HAS_SSE()) {
		mxcsr |= mask << _SSE_EMASK_SHIFT;
		__ldmxcsr(mxcsr);
	}
	return (omask);
}

__weak_reference(__feenableexcept, feenableexcept);
__weak_reference(__fedisableexcept, fedisableexcept);