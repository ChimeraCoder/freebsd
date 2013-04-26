
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
#include <sys/elf.h>
#include <sys/sysctl.h>
#include <sys/user.h>

#include <vm/vm.h>

#include <err.h>
#include <errno.h>
#include <libprocstat.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procstat.h"

#define	PRINT(name, spec, val)		\
	printf("%s %-16s " #spec "\n", prefix, #name, (val))
#define	PRINT_UNKNOWN(type, val)	\
	printf("%s %16ld %#lx\n", prefix, (long)type, (u_long)(val))

void
procstat_auxv(struct procstat *procstat, struct kinfo_proc *kipp)
{
	Elf_Auxinfo *auxv;
	u_int count, i;
	static char prefix[256];

	if (!hflag)
		printf("%5s %-16s %-16s %-16s\n", "PID", "COMM", "AUXV", "VALUE");
	auxv = procstat_getauxv(procstat, kipp, &count);
	if (auxv == NULL)
		return;
	snprintf(prefix, sizeof(prefix), "%5d %-16s", kipp->ki_pid,
	    kipp->ki_comm);
	for (i = 0; i < count; i++) {
		switch(auxv[i].a_type) {
		case AT_NULL:
			return;
		case AT_IGNORE:
			break;
		case AT_EXECFD:
			PRINT(AT_EXECFD, %ld, (long)auxv[i].a_un.a_val);
			break;
		case AT_PHDR:
			PRINT(AT_PHDR, %p, auxv[i].a_un.a_ptr);
			break;
		case AT_PHENT:
			PRINT(AT_PHENT, %ld, (long)auxv[i].a_un.a_val);
			break;
		case AT_PHNUM:
			PRINT(AT_PHNUM, %ld, (long)auxv[i].a_un.a_val);
			break;
		case AT_PAGESZ:
			PRINT(AT_PAGESZ, %ld, (long)auxv[i].a_un.a_val);
			break;
		case AT_BASE:
			PRINT(AT_BASE, %p, auxv[i].a_un.a_ptr);
			break;
		case AT_FLAGS:
			PRINT(AT_FLAGS, %#lx, (u_long)auxv[i].a_un.a_val);
			break;
		case AT_ENTRY:
			PRINT(AT_ENTRY, %p, auxv[i].a_un.a_ptr);
			break;
#ifdef AT_NOTELF
		case AT_NOTELF:
			PRINT(AT_NOTELF, %ld, (long)auxv[i].a_un.a_val);
			break;
#endif
#ifdef AT_UID
		case AT_UID:
			PRINT(AT_UID, %ld, (long)auxv[i].a_un.a_val);
			break;
#endif
#ifdef AT_EUID
		case AT_EUID:
			PRINT(AT_EUID, %ld, (long)auxv[i].a_un.a_val);
			break;
#endif
#ifdef AT_GID
		case AT_GID:
			PRINT(AT_GID, %ld, (long)auxv[i].a_un.a_val);
			break;
#endif
#ifdef AT_EGID
		case AT_EGID:
			PRINT(AT_EGID, %ld, (long)auxv[i].a_un.a_val);
			break;
#endif
		case AT_EXECPATH:
			PRINT(AT_EXECPATH, %p, auxv[i].a_un.a_ptr);
			break;
		case AT_CANARY:
			PRINT(AT_CANARY, %p, auxv[i].a_un.a_ptr);
			break;
		case AT_CANARYLEN:
			PRINT(AT_CANARYLEN, %ld, (long)auxv[i].a_un.a_val);
			break;
		case AT_OSRELDATE:
			PRINT(AT_OSRELDATE, %ld, (long)auxv[i].a_un.a_val);
			break;
		case AT_NCPUS:
			PRINT(AT_NCPUS, %ld, (long)auxv[i].a_un.a_val);
			break;
		case AT_PAGESIZES:
			PRINT(AT_PAGESIZES, %p, auxv[i].a_un.a_ptr);
			break;
		case AT_PAGESIZESLEN:
			PRINT(AT_PAGESIZESLEN, %ld, (long)auxv[i].a_un.a_val);
			break;
		case AT_STACKPROT:
			if ((auxv[i].a_un.a_val & VM_PROT_EXECUTE) != 0)
				PRINT(AT_STACKPROT, %s, "NONEXECUTABLE");
			else
				PRINT(AT_STACKPROT, %s, "EXECUTABLE");
			break;
#ifdef AT_TIMEKEEP
		case AT_TIMEKEEP:
			PRINT(AT_TIMEKEEP, %p, auxv[i].a_un.a_ptr);
			break;
#endif
		default:
			PRINT_UNKNOWN(auxv[i].a_type, auxv[i].a_un.a_val);
			break;
		}
	}
	printf("\n");
	procstat_freeauxv(procstat, auxv);
}