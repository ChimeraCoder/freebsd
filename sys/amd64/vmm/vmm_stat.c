
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/smp.h>

#include <machine/vmm.h>
#include "vmm_util.h"
#include "vmm_stat.h"

static int vstnum;
static struct vmm_stat_type *vsttab[MAX_VMM_STAT_TYPES];

static MALLOC_DEFINE(M_VMM_STAT, "vmm stat", "vmm stat");

void
vmm_stat_init(void *arg)
{
	struct vmm_stat_type *vst = arg;

	/* We require all stats to identify themselves with a description */
	if (vst->desc == NULL)
		return;

	if (vst->scope == VMM_STAT_SCOPE_INTEL && !vmm_is_intel())
		return;

	if (vst->scope == VMM_STAT_SCOPE_AMD && !vmm_is_amd())
		return;

	if (vstnum >= MAX_VMM_STAT_TYPES) {
		printf("Cannot accomodate vmm stat type \"%s\"!\n", vst->desc);
		return;
	}

	vst->index = vstnum;
	vsttab[vstnum++] = vst;
}

int
vmm_stat_copy(struct vm *vm, int vcpu, int *num_stats, uint64_t *buf)
{
	int i;
	uint64_t *stats;

	if (vcpu < 0 || vcpu >= VM_MAXCPU)
		return (EINVAL);
		
	stats = vcpu_stats(vm, vcpu);
	for (i = 0; i < vstnum; i++)
		buf[i] = stats[i];
	*num_stats = vstnum;
	return (0);
}

void *
vmm_stat_alloc(void)
{
	u_long size;
	
	size = vstnum * sizeof(uint64_t);

	return (malloc(size, M_VMM_STAT, M_ZERO | M_WAITOK));
}

void
vmm_stat_free(void *vp)
{
	free(vp, M_VMM_STAT);
}

const char *
vmm_stat_desc(int index)
{

	if (index >= 0 && index < vstnum)
		return (vsttab[index]->desc);
	else
		return (NULL);
}

/* global statistics */
VMM_STAT(VCPU_MIGRATIONS, "vcpu migration across host cpus");
VMM_STAT(VMEXIT_COUNT, "total number of vm exits");
VMM_STAT(VMEXIT_EXTINT, "vm exits due to external interrupt");
VMM_STAT(VMEXIT_HLT, "number of times hlt was intercepted");
VMM_STAT(VMEXIT_CR_ACCESS, "number of times %cr access was intercepted");
VMM_STAT(VMEXIT_RDMSR, "number of times rdmsr was intercepted");
VMM_STAT(VMEXIT_WRMSR, "number of times wrmsr was intercepted");
VMM_STAT(VMEXIT_MTRAP, "number of monitor trap exits");
VMM_STAT(VMEXIT_PAUSE, "number of times pause was intercepted");
VMM_STAT(VMEXIT_INTR_WINDOW, "vm exits due to interrupt window opening");
VMM_STAT(VMEXIT_NMI_WINDOW, "vm exits due to nmi window opening");
VMM_STAT(VMEXIT_INOUT, "number of times in/out was intercepted");
VMM_STAT(VMEXIT_CPUID, "number of times cpuid was intercepted");
VMM_STAT(VMEXIT_EPT_FAULT, "vm exits due to nested page fault");
VMM_STAT(VMEXIT_UNKNOWN, "number of vm exits for unknown reason");
VMM_STAT(VMEXIT_ASTPENDING, "number of times astpending at exit");
VMM_STAT(VMEXIT_USERSPACE, "number of vm exits handled in userspace");