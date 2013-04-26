
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

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>

#include <err.h>
#include <errno.h>
#include <libprocstat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libutil.h>

#include "procstat.h"

void
procstat_vm(struct procstat *procstat, struct kinfo_proc *kipp)
{
	struct kinfo_vmentry *freep, *kve;
	int ptrwidth;
	int i, cnt;
	const char *str;

	ptrwidth = 2*sizeof(void *) + 2;
	if (!hflag)
		printf("%5s %*s %*s %3s %4s %4s %3s %3s %4s %-2s %-s\n",
		    "PID", ptrwidth, "START", ptrwidth, "END", "PRT", "RES",
		    "PRES", "REF", "SHD", "FL", "TP", "PATH");

	freep = procstat_getvmmap(procstat, kipp, &cnt);
	if (freep == NULL)
		return;
	for (i = 0; i < cnt; i++) {
		kve = &freep[i];
		printf("%5d ", kipp->ki_pid);
		printf("%#*jx ", ptrwidth, (uintmax_t)kve->kve_start);
		printf("%#*jx ", ptrwidth, (uintmax_t)kve->kve_end);
		printf("%s", kve->kve_protection & KVME_PROT_READ ? "r" : "-");
		printf("%s", kve->kve_protection & KVME_PROT_WRITE ? "w" : "-");
		printf("%s ", kve->kve_protection & KVME_PROT_EXEC ? "x" : "-");
		printf("%4d ", kve->kve_resident);
		printf("%4d ", kve->kve_private_resident);
		printf("%3d ", kve->kve_ref_count);
		printf("%3d ", kve->kve_shadow_count);
		printf("%-1s", kve->kve_flags & KVME_FLAG_COW ? "C" : "-");
		printf("%-1s", kve->kve_flags & KVME_FLAG_NEEDS_COPY ? "N" :
		    "-");
		printf("%-1s", kve->kve_flags & KVME_FLAG_SUPER ? "S" : "-");
		printf("%-1s ", kve->kve_flags & KVME_FLAG_GROWS_UP ? "U" :
		    kve->kve_flags & KVME_FLAG_GROWS_DOWN ? "D" : "-");
		switch (kve->kve_type) {
		case KVME_TYPE_NONE:
			str = "--";
			break;
		case KVME_TYPE_DEFAULT:
			str = "df";
			break;
		case KVME_TYPE_VNODE:
			str = "vn";
			break;
		case KVME_TYPE_SWAP:
			str = "sw";
			break;
		case KVME_TYPE_DEVICE:
			str = "dv";
			break;
		case KVME_TYPE_PHYS:
			str = "ph";
			break;
		case KVME_TYPE_DEAD:
			str = "dd";
			break;
		case KVME_TYPE_SG:
			str = "sg";
			break;
		case KVME_TYPE_UNKNOWN:
		default:
			str = "??";
			break;
		}
		printf("%-2s ", str);
		printf("%-s\n", kve->kve_path);
	}
	free(freep);
}