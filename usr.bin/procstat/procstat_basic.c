
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
#include <libprocstat.h>
#include <stdio.h>
#include <string.h>

#include "procstat.h"

void
procstat_basic(struct kinfo_proc *kipp)
{

	if (!hflag)
		printf("%5s %5s %5s %5s %5s %3s %-8s %-9s %-13s %-12s\n",
		    "PID", "PPID", "PGID", "SID", "TSID", "THR", "LOGIN",
		    "WCHAN", "EMUL", "COMM");

	printf("%5d ", kipp->ki_pid);
	printf("%5d ", kipp->ki_ppid);
	printf("%5d ", kipp->ki_pgid);
	printf("%5d ", kipp->ki_sid);
	printf("%5d ", kipp->ki_tsid);
	printf("%3d ", kipp->ki_numthreads);
	printf("%-8s ", strlen(kipp->ki_login) ? kipp->ki_login : "-");
	if (kipp->ki_kiflag & KI_LOCKBLOCK) {
		printf("*%-8s ", strlen(kipp->ki_lockname) ?
		    kipp->ki_lockname : "-");
	} else {
		printf("%-9s ", strlen(kipp->ki_wmesg) ?
		    kipp->ki_wmesg : "-");
	}
	printf("%-13s ", strcmp(kipp->ki_emul, "null") ? kipp->ki_emul : "-");
	printf("%-12s\n", kipp->ki_comm);
}