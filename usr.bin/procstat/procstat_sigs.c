
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

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libprocstat.h>

#include "procstat.h"

static void
procstat_print_signame(int sig)
{
	char name[12];
	int i;

	if (!nflag && sig < sys_nsig) {
		strlcpy(name, sys_signame[sig], sizeof(name));
		for (i = 0; name[i] != 0; i++)
			name[i] = toupper(name[i]);
		printf("%-7s ", name);
	} else
		printf("%-7d ", sig);
}

static void
procstat_print_sig(const sigset_t *set, int sig, char flag)
{

	printf("%c", sigismember(set, sig) ? flag : '-');
}

void
procstat_sigs(struct procstat *prstat __unused, struct kinfo_proc *kipp)
{
	int j;
	pid_t pid;

	pid = kipp->ki_pid;
	if (!hflag)
		printf("%5s %-16s %-7s %4s\n", "PID", "COMM", "SIG", "FLAGS");

	for (j = 1; j <= _SIG_MAXSIG; j++) {
		printf("%5d ", pid);
		printf("%-16s ", kipp->ki_comm);
		procstat_print_signame(j);
		printf(" ");
		procstat_print_sig(&kipp->ki_siglist, j, 'P');
		procstat_print_sig(&kipp->ki_sigignore, j, 'I');
		procstat_print_sig(&kipp->ki_sigcatch, j, 'C');
		printf("\n");
	}
}

void
procstat_threads_sigs(struct procstat *procstat, struct kinfo_proc *kipp)
{
	struct kinfo_proc *kip;
	pid_t pid;
	int j;
	unsigned int count, i;

	pid = kipp->ki_pid;
	if (!hflag)
		printf("%5s %6s %-16s %-7s %4s\n", "PID", "TID", "COMM",
		     "SIG", "FLAGS");

	kip = procstat_getprocs(procstat, KERN_PROC_PID | KERN_PROC_INC_THREAD,
	    pid, &count);
	if (kip == NULL)
		return;
	kinfo_proc_sort(kip, count);
	for (i = 0; i < count; i++) {
		kipp = &kip[i];
		for (j = 1; j <= _SIG_MAXSIG; j++) {
			printf("%5d ", pid);
			printf("%6d ", kipp->ki_tid);
			printf("%-16s ", kipp->ki_comm);
			procstat_print_signame(j);
			printf(" ");
			procstat_print_sig(&kipp->ki_siglist, j, 'P');
			procstat_print_sig(&kipp->ki_sigmask, j, 'B');
			printf("\n");
		}
	}
	procstat_freeprocs(procstat, kip);
}