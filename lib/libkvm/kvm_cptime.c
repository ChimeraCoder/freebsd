
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
#include <sys/pcpu.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <errno.h>
#include <kvm.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "kvm_private.h"

static struct nlist kvm_cp_time_nl[] = {
	{ .n_name = "_cp_time" },		/* (deprecated) */
	{ .n_name = NULL },
};

#define	NL_CP_TIME		0

static int kvm_cp_time_cached;

static int
_kvm_cp_time_init(kvm_t *kd)
{

	if (kvm_nlist(kd, kvm_cp_time_nl) < 0)
		return (-1);
	kvm_cp_time_cached = 1;
	return (0);
}

static int
getsysctl(kvm_t *kd, const char *name, void *buf, size_t len)
{
	size_t nlen;

	nlen = len;
	if (sysctlbyname(name, buf, &nlen, NULL, 0) < 0) {
		_kvm_err(kd, kd->program, "cannot read sysctl %s:%s", name,
		    strerror(errno));
		return (-1);
	}
	if (nlen != len) {
		_kvm_err(kd, kd->program, "sysctl %s has unexpected size",
		    name);
		return (-1);
	}
	return (0);
}

int
kvm_getcptime(kvm_t *kd, long *cp_time)
{
	struct pcpu *pc;
	int i, j, maxcpu;

	if (kd == NULL) {
		kvm_cp_time_cached = 0;
		return (0);
	}

	if (ISALIVE(kd))
		return (getsysctl(kd, "kern.cp_time", cp_time, sizeof(long) *
		    CPUSTATES));

	if (kvm_cp_time_cached == 0) {
		if (_kvm_cp_time_init(kd) < 0)
			return (-1);
	}

	/* If this kernel has a "cp_time[]" symbol, then just read that. */
	if (kvm_cp_time_nl[NL_CP_TIME].n_value != 0) {
		if (kvm_read(kd, kvm_cp_time_nl[NL_CP_TIME].n_value, cp_time,
		    sizeof(long) * CPUSTATES) != sizeof(long) * CPUSTATES) {
			_kvm_err(kd, kd->program, "cannot read cp_time array");
			return (-1);
		}
		return (0);
	}

	/*
	 * If we don't have that symbol, then we have to simulate
	 * "cp_time[]" by adding up the individual times for each CPU.
	 */
	maxcpu = kvm_getmaxcpu(kd);
	if (maxcpu < 0)
		return (-1);
	for (i = 0; i < CPUSTATES; i++)
		cp_time[i] = 0;
	for (i = 0; i < maxcpu; i++) {
		pc = kvm_getpcpu(kd, i);
		if (pc == NULL)
			continue;
		if (pc == (void *)-1)
			return (-1);
		for (j = 0; j < CPUSTATES; j++)
			cp_time[j] += pc->pc_cp_time[j];
		free(pc);
	}
	return (0);
}