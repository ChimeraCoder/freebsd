
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
#include <string.h>

#include "procstat.h"

void
procstat_threads(struct procstat *procstat, struct kinfo_proc *kipp)
{
	struct kinfo_proc *kip;
	unsigned int count, i;
	const char *str;

	if (!hflag)
		printf("%5s %6s %-16s %-16s %2s %4s %-7s %-9s\n", "PID",
		    "TID", "COMM", "TDNAME", "CPU", "PRI", "STATE", "WCHAN");

	kip = procstat_getprocs(procstat, KERN_PROC_PID | KERN_PROC_INC_THREAD,
	    kipp->ki_pid, &count);
	if (kip == NULL)
		return;
	kinfo_proc_sort(kip, count);
	for (i = 0; i < count; i++) {
		kipp = &kip[i];
		printf("%5d ", kipp->ki_pid);
		printf("%6d ", kipp->ki_tid);
		printf("%-16s ", strlen(kipp->ki_comm) ?
		    kipp->ki_comm : "-");
		printf("%-16s ", (strlen(kipp->ki_tdname) &&
		    (strcmp(kipp->ki_comm, kipp->ki_tdname) != 0)) ?
		    kipp->ki_tdname : "-");
		if (kipp->ki_oncpu != 255)
			printf("%3d ", kipp->ki_oncpu);
		else if (kipp->ki_lastcpu != 255)
			printf("%3d ", kipp->ki_lastcpu);
		else
			printf("%3s ", "-");
		printf("%4d ", kipp->ki_pri.pri_level);
		switch (kipp->ki_stat) {
		case SRUN:
			str = "run";
			break;

		case SSTOP:
			str = "stop";
			break;

		case SSLEEP:
			str = "sleep";
			break;

		case SLOCK:
			str = "lock";
			break;

		case SWAIT:
			str = "wait";
			break;

		case SZOMB:
			str = "zomb";
			break;

		case SIDL:
			str = "idle";
			break;

		default:
			str = "??";
			break;
		}
		printf("%-7s ", str);
		if (kipp->ki_kiflag & KI_LOCKBLOCK) {
			printf("*%-8s ", strlen(kipp->ki_lockname) ?
			    kipp->ki_lockname : "-");
		} else {
			printf("%-9s ", strlen(kipp->ki_wmesg) ?
			    kipp->ki_wmesg : "-");
		}
		printf("\n");
	}
	procstat_freeprocs(procstat, kip);
}