
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/stack.h>
#include <sys/sysent.h>
#include <sys/pcpu.h>

#include <machine/frame.h>
#include <machine/md_var.h>
#include <machine/reg.h>
#include <machine/stack.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>

#include "regset.h"

/* Offset to the LR Save word (ppc32) */
#define RETURN_OFFSET	4
#define RETURN_OFFSET64	8

#define INKERNEL(x)	((x) <= VM_MAX_KERNEL_ADDRESS && \
		(x) >= VM_MIN_KERNEL_ADDRESS)

greg_t
dtrace_getfp(void)
{
	return (greg_t)__builtin_frame_address(0);
}

void
dtrace_getpcstack(pc_t *pcstack, int pcstack_limit, int aframes,
    uint32_t *intrpc)
{
	int depth = 0;
	register_t sp;
	vm_offset_t callpc;
	pc_t caller = (pc_t) solaris_cpu[curcpu].cpu_dtrace_caller;

	if (intrpc != 0)
		pcstack[depth++] = (pc_t) intrpc;

	aframes++;

	sp = dtrace_getfp();

	while (depth < pcstack_limit) {
		if (!INKERNEL((long) sp))
			break;

		callpc = *(uintptr_t *)(sp + RETURN_OFFSET);

		if (!INKERNEL(callpc))
			break;

		if (aframes > 0) {
			aframes--;
			if ((aframes == 0) && (caller != 0)) {
				pcstack[depth++] = caller;
			}
		}
		else {
			pcstack[depth++] = callpc;
		}

		sp = *(uintptr_t*)sp;
	}

	for (; depth < pcstack_limit; depth++) {
		pcstack[depth] = 0;
	}
}

static int
dtrace_getustack_common(uint64_t *pcstack, int pcstack_limit, uintptr_t pc,
    uintptr_t sp)
{
	proc_t *p = curproc;
	int ret = 0;

	ASSERT(pcstack == NULL || pcstack_limit > 0);

	while (pc != 0) {
		ret++;
		if (pcstack != NULL) {
			*pcstack++ = (uint64_t)pc;
			pcstack_limit--;
			if (pcstack_limit <= 0)
				break;
		}

		if (sp == 0)
			break;

		if (SV_PROC_FLAG(p, SV_ILP32)) {
			pc = dtrace_fuword32((void *)(sp + RETURN_OFFSET));
			sp = dtrace_fuword32((void *)sp);
		}
		else {
			pc = dtrace_fuword64((void *)(sp + RETURN_OFFSET64));
			sp = dtrace_fuword64((void *)sp);
		}
	}

	return (ret);
}

void
dtrace_getupcstack(uint64_t *pcstack, int pcstack_limit)
{
	proc_t *p = curproc;
	struct trapframe *tf;
	uintptr_t pc, sp;
	volatile uint16_t *flags =
	    (volatile uint16_t *)&cpu_core[curcpu].cpuc_dtrace_flags;
	int n;

	if (*flags & CPU_DTRACE_FAULT)
		return;

	if (pcstack_limit <= 0)
		return;

	/*
	 * If there's no user context we still need to zero the stack.
	 */
	if (p == NULL || (tf = curthread->td_frame) == NULL)
		goto zero;

	*pcstack++ = (uint64_t)p->p_pid;
	pcstack_limit--;

	if (pcstack_limit <= 0)
		return;

	pc = tf->srr0;
	sp = tf->fixreg[1];

	if (DTRACE_CPUFLAG_ISSET(CPU_DTRACE_ENTRY)) {
		/* 
		 * In an entry probe.  The frame pointer has not yet been
		 * pushed (that happens in the function prologue).  The
		 * best approach is to add the current pc as a missing top
		 * of stack and back the pc up to the caller, which is stored
		 * at the current stack pointer address since the call 
		 * instruction puts it there right before the branch.
		 */

		*pcstack++ = (uint64_t)pc;
		pcstack_limit--;
		if (pcstack_limit <= 0)
			return;

		pc = tf->lr;
	}

	n = dtrace_getustack_common(pcstack, pcstack_limit, pc, sp);
	ASSERT(n >= 0);
	ASSERT(n <= pcstack_limit);

	pcstack += n;
	pcstack_limit -= n;

zero:
	while (pcstack_limit-- > 0)
		*pcstack++ = 0;
}

int
dtrace_getustackdepth(void)
{
	proc_t *p = curproc;
	struct trapframe *tf;
	uintptr_t pc, sp;
	int n = 0;

	if (p == NULL || (tf = curthread->td_frame) == NULL)
		return (0);

	if (DTRACE_CPUFLAG_ISSET(CPU_DTRACE_FAULT))
		return (-1);

	pc = tf->srr0;
	sp = tf->fixreg[1];

	if (DTRACE_CPUFLAG_ISSET(CPU_DTRACE_ENTRY)) {
		/* 
		 * In an entry probe.  The frame pointer has not yet been
		 * pushed (that happens in the function prologue).  The
		 * best approach is to add the current pc as a missing top
		 * of stack and back the pc up to the caller, which is stored
		 * at the current stack pointer address since the call 
		 * instruction puts it there right before the branch.
		 */

		if (SV_PROC_FLAG(p, SV_ILP32)) {
			pc = dtrace_fuword32((void *) sp);
		}
		else
			pc = dtrace_fuword64((void *) sp);
		n++;
	}

	n += dtrace_getustack_common(NULL, 0, pc, sp);

	return (n);
}

void
dtrace_getufpstack(uint64_t *pcstack, uint64_t *fpstack, int pcstack_limit)
{
	proc_t *p = curproc;
	struct trapframe *tf;
	uintptr_t pc, sp;
	volatile uint16_t *flags =
	    (volatile uint16_t *)&cpu_core[curcpu].cpuc_dtrace_flags;
#ifdef notyet	/* XXX signal stack */
	uintptr_t oldcontext;
	size_t s1, s2;
#endif

	if (*flags & CPU_DTRACE_FAULT)
		return;

	if (pcstack_limit <= 0)
		return;

	/*
	 * If there's no user context we still need to zero the stack.
	 */
	if (p == NULL || (tf = curthread->td_frame) == NULL)
		goto zero;

	*pcstack++ = (uint64_t)p->p_pid;
	pcstack_limit--;

	if (pcstack_limit <= 0)
		return;

	pc = tf->srr0;
	sp = tf->fixreg[1];

#ifdef notyet /* XXX signal stack */
	oldcontext = lwp->lwp_oldcontext;
	s1 = sizeof (struct xframe) + 2 * sizeof (long);
	s2 = s1 + sizeof (siginfo_t);
#endif

	if (DTRACE_CPUFLAG_ISSET(CPU_DTRACE_ENTRY)) {
		*pcstack++ = (uint64_t)pc;
		*fpstack++ = 0;
		pcstack_limit--;
		if (pcstack_limit <= 0)
			return;

		if (SV_PROC_FLAG(p, SV_ILP32)) {
			pc = dtrace_fuword32((void *)sp);
		}
		else {
			pc = dtrace_fuword64((void *)sp);
		}
	}

	while (pc != 0) {
		*pcstack++ = (uint64_t)pc;
		*fpstack++ = sp;
		pcstack_limit--;
		if (pcstack_limit <= 0)
			break;

		if (sp == 0)
			break;

#ifdef notyet /* XXX signal stack */
		if (oldcontext == sp + s1 || oldcontext == sp + s2) {
			ucontext_t *ucp = (ucontext_t *)oldcontext;
			greg_t *gregs = ucp->uc_mcontext.gregs;

			sp = dtrace_fulword(&gregs[REG_FP]);
			pc = dtrace_fulword(&gregs[REG_PC]);

			oldcontext = dtrace_fulword(&ucp->uc_link);
		} else
#endif /* XXX */
		{
			if (SV_PROC_FLAG(p, SV_ILP32)) {
				pc = dtrace_fuword32((void *)(sp + RETURN_OFFSET));
				sp = dtrace_fuword32((void *)sp);
			}
			else {
				pc = dtrace_fuword64((void *)(sp + RETURN_OFFSET64));
				sp = dtrace_fuword64((void *)sp);
			}
		}

		/*
		 * This is totally bogus:  if we faulted, we're going to clear
		 * the fault and break.  This is to deal with the apparently
		 * broken Java stacks on x86.
		 */
		if (*flags & CPU_DTRACE_FAULT) {
			*flags &= ~CPU_DTRACE_FAULT;
			break;
		}
	}

zero:
	while (pcstack_limit-- > 0)
		*pcstack++ = 0;
}

/*ARGSUSED*/
uint64_t
dtrace_getarg(int arg, int aframes)
{
	return (0);
}

#ifdef notyet
{
	int depth = 0;
	register_t sp;
	vm_offset_t callpc;
	pc_t caller = (pc_t) solaris_cpu[curcpu].cpu_dtrace_caller;

	if (intrpc != 0)
		pcstack[depth++] = (pc_t) intrpc;

	aframes++;

	sp = dtrace_getfp();

	while (depth < pcstack_limit) {
		if (!INKERNEL((long) frame))
			break;

		callpc = *(void **)(sp + RETURN_OFFSET);

		if (!INKERNEL(callpc))
			break;

		if (aframes > 0) {
			aframes--;
			if ((aframes == 0) && (caller != 0)) {
				pcstack[depth++] = caller;
			}
		}
		else {
			pcstack[depth++] = callpc;
		}

		sp = *(void **)sp;
	}

	for (; depth < pcstack_limit; depth++) {
		pcstack[depth] = 0;
	}
}
#endif

int
dtrace_getstackdepth(int aframes)
{
	int depth = 0;
	register_t sp;

	aframes++;
	sp = dtrace_getfp();
	depth++;
	for(;;) {
		if (!INKERNEL((long) sp))
			break;
		if (!INKERNEL((long) *(void **)sp))
			break;
		depth++;
		sp = *(uintptr_t *)sp;
	}
	if (depth < aframes)
		return 0;
	else
		return depth - aframes;
}

ulong_t
dtrace_getreg(struct trapframe *rp, uint_t reg)
{
	if (reg < 32)
		return (rp->fixreg[reg]);

	switch (reg) {
	case 33:
		return (rp->lr);
	case 34:
		return (rp->cr);
	case 35:
		return (rp->xer);
	case 36:
		return (rp->ctr);
	case 37:
		return (rp->srr0);
	case 38:
		return (rp->srr1);
	case 39:
		return (rp->exc);
	default:
		DTRACE_CPUFLAG_SET(CPU_DTRACE_ILLOP);
		return (0);
	}
}

static int
dtrace_copycheck(uintptr_t uaddr, uintptr_t kaddr, size_t size)
{
	ASSERT(INKERNEL(kaddr) && kaddr + size >= kaddr);

	if (uaddr + size > VM_MAXUSER_ADDRESS || uaddr + size < uaddr) {
		DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
		cpu_core[curcpu].cpuc_dtrace_illval = uaddr;
		return (0);
	}

	return (1);
}

void
dtrace_copyin(uintptr_t uaddr, uintptr_t kaddr, size_t size,
    volatile uint16_t *flags)
{
	if (dtrace_copycheck(uaddr, kaddr, size))
		if (copyin((const void *)uaddr, (void *)kaddr, size)) {
			DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
			cpu_core[curcpu].cpuc_dtrace_illval = (uintptr_t)uaddr;
		}
}

void
dtrace_copyout(uintptr_t kaddr, uintptr_t uaddr, size_t size,
    volatile uint16_t *flags)
{
	if (dtrace_copycheck(uaddr, kaddr, size)) {
		if (copyout((const void *)kaddr, (void *)uaddr, size)) {
			DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
			cpu_core[curcpu].cpuc_dtrace_illval = (uintptr_t)uaddr;
		}
	}
}

void
dtrace_copyinstr(uintptr_t uaddr, uintptr_t kaddr, size_t size,
    volatile uint16_t *flags)
{
	size_t actual;
	int    error;

	if (dtrace_copycheck(uaddr, kaddr, size)) {
		error = copyinstr((const void *)uaddr, (void *)kaddr,
		    size, &actual);
		
		/* ENAMETOOLONG is not a fault condition. */
		if (error && error != ENAMETOOLONG) {
			DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
			cpu_core[curcpu].cpuc_dtrace_illval = (uintptr_t)uaddr;
		}
	}
}

/*
 * The bulk of this function could be replaced to match dtrace_copyinstr() 
 * if we ever implement a copyoutstr().
 */
void
dtrace_copyoutstr(uintptr_t kaddr, uintptr_t uaddr, size_t size,
    volatile uint16_t *flags)
{
	size_t len;

	if (dtrace_copycheck(uaddr, kaddr, size)) {
		len = strlen((const char *)kaddr);
		if (len > size)
			len = size;

		if (copyout((const void *)kaddr, (void *)uaddr, len)) {
			DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
			cpu_core[curcpu].cpuc_dtrace_illval = (uintptr_t)uaddr;
		}
	}
}

uint8_t
dtrace_fuword8(void *uaddr)
{
	if ((uintptr_t)uaddr > VM_MAXUSER_ADDRESS) {
		DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
		cpu_core[curcpu].cpuc_dtrace_illval = (uintptr_t)uaddr;
		return (0);
	}
	return (fubyte(uaddr));
}

uint16_t
dtrace_fuword16(void *uaddr)
{
	uint16_t ret = 0;

	if (dtrace_copycheck((uintptr_t)uaddr, (uintptr_t)&ret, sizeof(ret))) {
		if (copyin((const void *)uaddr, (void *)&ret, sizeof(ret))) {
			DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
			cpu_core[curcpu].cpuc_dtrace_illval = (uintptr_t)uaddr;
		}
	}
	return ret;
}

uint32_t
dtrace_fuword32(void *uaddr)
{
	if ((uintptr_t)uaddr > VM_MAXUSER_ADDRESS) {
		DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
		cpu_core[curcpu].cpuc_dtrace_illval = (uintptr_t)uaddr;
		return (0);
	}
	return (fuword32(uaddr));
}

uint64_t
dtrace_fuword64(void *uaddr)
{
	uint64_t ret = 0;

	if (dtrace_copycheck((uintptr_t)uaddr, (uintptr_t)&ret, sizeof(ret))) {
		if (copyin((const void *)uaddr, (void *)&ret, sizeof(ret))) {
			DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
			cpu_core[curcpu].cpuc_dtrace_illval = (uintptr_t)uaddr;
		}
	}
	return ret;
}

uintptr_t
dtrace_fulword(void *uaddr)
{
	uintptr_t ret = 0;

	if (dtrace_copycheck((uintptr_t)uaddr, (uintptr_t)&ret, sizeof(ret))) {
		if (copyin((const void *)uaddr, (void *)&ret, sizeof(ret))) {
			DTRACE_CPUFLAG_SET(CPU_DTRACE_BADADDR);
			cpu_core[curcpu].cpuc_dtrace_illval = (uintptr_t)uaddr;
		}
	}
	return ret;
}