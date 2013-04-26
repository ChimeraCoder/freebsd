
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

#include "opt_cpu.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/pcpu.h>
#include <sys/systm.h>
#include <sys/sysctl.h>

#include <machine/cputypes.h>
#include <machine/md_var.h>
#include <machine/specialreg.h>

#include <vm/vm.h>
#include <vm/pmap.h>

static int	hw_instruction_sse;
SYSCTL_INT(_hw, OID_AUTO, instruction_sse, CTLFLAG_RD,
    &hw_instruction_sse, 0, "SIMD/MMX2 instructions available in CPU");
/*
 * -1: automatic (default)
 *  0: keep enable CLFLUSH
 *  1: force disable CLFLUSH
 */
static int	hw_clflush_disable = -1;

int	cpu;			/* Are we 386, 386sx, 486, etc? */
u_int	cpu_feature;		/* Feature flags */
u_int	cpu_feature2;		/* Feature flags */
u_int	amd_feature;		/* AMD feature flags */
u_int	amd_feature2;		/* AMD feature flags */
u_int	amd_pminfo;		/* AMD advanced power management info */
u_int	via_feature_rng;	/* VIA RNG features */
u_int	via_feature_xcrypt;	/* VIA ACE features */
u_int	cpu_high;		/* Highest arg to CPUID */
u_int	cpu_exthigh;		/* Highest arg to extended CPUID */
u_int	cpu_id;			/* Stepping ID */
u_int	cpu_procinfo;		/* HyperThreading Info / Brand Index / CLFUSH */
u_int	cpu_procinfo2;		/* Multicore info */
char	cpu_vendor[20];		/* CPU Origin code */
u_int	cpu_vendor_id;		/* CPU vendor ID */
u_int	cpu_fxsr;		/* SSE enabled */
u_int	cpu_mxcsr_mask;		/* Valid bits in mxcsr */
u_int	cpu_clflush_line_size = 32;
u_int	cpu_stdext_feature;
u_int	cpu_max_ext_state_size;

SYSCTL_UINT(_hw, OID_AUTO, via_feature_rng, CTLFLAG_RD,
	&via_feature_rng, 0, "VIA RNG feature available in CPU");
SYSCTL_UINT(_hw, OID_AUTO, via_feature_xcrypt, CTLFLAG_RD,
	&via_feature_xcrypt, 0, "VIA xcrypt feature available in CPU");

static void
init_amd(void)
{

	/*
	 * Work around Erratum 721 for Family 10h and 12h processors.
	 * These processors may incorrectly update the stack pointer
	 * after a long series of push and/or near-call instructions,
	 * or a long series of pop and/or near-return instructions.
	 *
	 * http://support.amd.com/us/Processor_TechDocs/41322_10h_Rev_Gd.pdf
	 * http://support.amd.com/us/Processor_TechDocs/44739_12h_Rev_Gd.pdf
	 *
	 * Hypervisors do not provide access to the errata MSR,
	 * causing #GP exception on attempt to apply the errata.  The
	 * MSR write shall be done on host and persist globally
	 * anyway, so do not try to do it when under virtualization.
	 */
	switch (CPUID_TO_FAMILY(cpu_id)) {
	case 0x10:
	case 0x12:
		if ((cpu_feature2 & CPUID2_HV) == 0)
			wrmsr(0xc0011029, rdmsr(0xc0011029) | 1);
		break;
	}
}

/*
 * Initialize special VIA features
 */
static void
init_via(void)
{
	u_int regs[4], val;

	/*
	 * Check extended CPUID for PadLock features.
	 *
	 * http://www.via.com.tw/en/downloads/whitepapers/initiatives/padlock/programming_guide.pdf
	 */
	do_cpuid(0xc0000000, regs);
	if (regs[0] >= 0xc0000001) {
		do_cpuid(0xc0000001, regs);
		val = regs[3];
	} else
		return;

	/* Enable RNG if present. */
	if ((val & VIA_CPUID_HAS_RNG) != 0) {
		via_feature_rng = VIA_HAS_RNG;
		wrmsr(0x110B, rdmsr(0x110B) | VIA_CPUID_DO_RNG);
	}

	/* Enable PadLock if present. */
	if ((val & VIA_CPUID_HAS_ACE) != 0)
		via_feature_xcrypt |= VIA_HAS_AES;
	if ((val & VIA_CPUID_HAS_ACE2) != 0)
		via_feature_xcrypt |= VIA_HAS_AESCTR;
	if ((val & VIA_CPUID_HAS_PHE) != 0)
		via_feature_xcrypt |= VIA_HAS_SHA;
	if ((val & VIA_CPUID_HAS_PMM) != 0)
		via_feature_xcrypt |= VIA_HAS_MM;
	if (via_feature_xcrypt != 0)
		wrmsr(0x1107, rdmsr(0x1107) | (1 << 28));
}

/*
 * Initialize CPU control registers
 */
void
initializecpu(void)
{
	uint64_t msr;
	uint32_t cr4;

	cr4 = rcr4();
	if ((cpu_feature & CPUID_XMM) && (cpu_feature & CPUID_FXSR)) {
		cr4 |= CR4_FXSR | CR4_XMM;
		cpu_fxsr = hw_instruction_sse = 1;
	}
	if (cpu_stdext_feature & CPUID_STDEXT_FSGSBASE)
		cr4 |= CR4_FSGSBASE;

	/*
	 * Postpone enabling the SMEP on the boot CPU until the page
	 * tables are switched from the boot loader identity mapping
	 * to the kernel tables.  The boot loader enables the U bit in
	 * its tables.
	 */
	if (!IS_BSP() && (cpu_stdext_feature & CPUID_STDEXT_SMEP))
		cr4 |= CR4_SMEP;
	load_cr4(cr4);
	if ((amd_feature & AMDID_NX) != 0) {
		msr = rdmsr(MSR_EFER) | EFER_NXE;
		wrmsr(MSR_EFER, msr);
		pg_nx = PG_NX;
	}
	switch (cpu_vendor_id) {
	case CPU_VENDOR_AMD:
		init_amd();
		break;
	case CPU_VENDOR_CENTAUR:
		init_via();
		break;
	}
}

void
initializecpucache()
{

	/*
	 * CPUID with %eax = 1, %ebx returns
	 * Bits 15-8: CLFLUSH line size
	 * 	(Value * 8 = cache line size in bytes)
	 */
	if ((cpu_feature & CPUID_CLFSH) != 0)
		cpu_clflush_line_size = ((cpu_procinfo >> 8) & 0xff) * 8;
	/*
	 * XXXKIB: (temporary) hack to work around traps generated
	 * when CLFLUSHing APIC register window under virtualization
	 * environments.  These environments tend to disable the
	 * CPUID_SS feature even though the native CPU supports it.
	 */
	TUNABLE_INT_FETCH("hw.clflush_disable", &hw_clflush_disable);
	if (vm_guest != VM_GUEST_NO && hw_clflush_disable == -1)
		cpu_feature &= ~CPUID_CLFSH;
	/*
	 * Allow to disable CLFLUSH feature manually by
	 * hw.clflush_disable tunable.
	 */
	if (hw_clflush_disable == 1)
		cpu_feature &= ~CPUID_CLFSH;
}