
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

#ifdef _KERNEL
#include "opt_bpf.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/sysctl.h>
#else
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/types.h>
#endif

#include <net/bpf.h>
#include <net/bpf_jitter.h>

bpf_filter_func	bpf_jit_compile(struct bpf_insn *, u_int, size_t *);

static u_int	bpf_jit_accept_all(u_char *, u_int, u_int);

#ifdef _KERNEL
MALLOC_DEFINE(M_BPFJIT, "BPF_JIT", "BPF JIT compiler");

SYSCTL_NODE(_net, OID_AUTO, bpf_jitter, CTLFLAG_RW, 0, "BPF JIT compiler");
int bpf_jitter_enable = 1;
SYSCTL_INT(_net_bpf_jitter, OID_AUTO, enable, CTLFLAG_RW,
    &bpf_jitter_enable, 0, "enable BPF JIT compiler");
#endif

bpf_jit_filter *
bpf_jitter(struct bpf_insn *fp, int nins)
{
	bpf_jit_filter *filter;

	/* Allocate the filter structure. */
#ifdef _KERNEL
	filter = (struct bpf_jit_filter *)malloc(sizeof(*filter),
	    M_BPFJIT, M_NOWAIT);
#else
	filter = (struct bpf_jit_filter *)malloc(sizeof(*filter));
#endif
	if (filter == NULL)
		return (NULL);

	/* No filter means accept all. */
	if (fp == NULL || nins == 0) {
		filter->func = bpf_jit_accept_all;
		return (filter);
	}

	/* Create the binary. */
	if ((filter->func = bpf_jit_compile(fp, nins, &filter->size)) == NULL) {
#ifdef _KERNEL
		free(filter, M_BPFJIT);
#else
		free(filter);
#endif
		return (NULL);
	}

	return (filter);
}

void
bpf_destroy_jit_filter(bpf_jit_filter *filter)
{

#ifdef _KERNEL
	if (filter->func != bpf_jit_accept_all)
		free(filter->func, M_BPFJIT);
	free(filter, M_BPFJIT);
#else
	if (filter->func != bpf_jit_accept_all)
		munmap(filter->func, filter->size);
	free(filter);
#endif
}

static u_int
bpf_jit_accept_all(__unused u_char *p, __unused u_int wirelen,
    __unused u_int buflen)
{

	return ((u_int)-1);
}