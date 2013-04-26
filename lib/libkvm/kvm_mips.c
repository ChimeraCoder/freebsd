
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

/*
 * MIPS machine dependent routines for kvm.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/elf32.h>
#include <sys/mman.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>

#include <machine/pmap.h>

#include <db.h>
#include <limits.h>
#include <kvm.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kvm_private.h"

/* minidump must be the first item! */
struct vmstate {
	int minidump;		/* 1 = minidump mode */
	void *mmapbase;
	size_t mmapsize;
};

void
_kvm_freevtop(kvm_t *kd)
{
	if (kd->vmst != 0) {
		if (kd->vmst->minidump)
			return (_kvm_minidump_freevtop(kd));
		if (kd->vmst->mmapbase != NULL)
			munmap(kd->vmst->mmapbase, kd->vmst->mmapsize);
		free(kd->vmst);
		kd->vmst = NULL;
	}
}

int
_kvm_initvtop(kvm_t *kd)
{
	char minihdr[8];

	if (!kd->rawdump) {
		if (pread(kd->pmfd, &minihdr, 8, 0) == 8) {
			if (memcmp(&minihdr, "minidump", 8) == 0)
				return (_kvm_minidump_initvtop(kd));
		} else {
			_kvm_err(kd, kd->program, "cannot read header");
			return (-1);
		}
	}

	_kvm_err(kd, 0, "_kvm_initvtop: Unsupported image type");
	return (-1);
}

int
_kvm_kvatop(kvm_t *kd, u_long va, off_t *pa)
{

	if (kd->vmst->minidump)
		return _kvm_minidump_kvatop(kd, va, pa);


	_kvm_err(kd, 0, "_kvm_kvatop: Unsupported image type");
	return (0);
}

/*
 * Machine-dependent initialization for ALL open kvm descriptors,
 * not just those for a kernel crash dump.  Some architectures
 * have to deal with these NOT being constants!  (i.e. m68k)
 */
#ifdef FBSD_NOT_YET
int
_kvm_mdopen(kvm_t *kd __unused)
{

	return (0);
}
#endif