
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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "procstat.h"

static const char *get_umask(struct procstat *procstat,
    struct kinfo_proc *kipp);

void
procstat_cred(struct procstat *procstat, struct kinfo_proc *kipp)
{
	unsigned int i, ngroups;
	gid_t *groups;

	if (!hflag)
		printf("%5s %-16s %5s %5s %5s %5s %5s %5s %5s %5s %-15s\n",
		    "PID", "COMM", "EUID", "RUID", "SVUID", "EGID", "RGID",
		    "SVGID", "UMASK", "FLAGS", "GROUPS");

	printf("%5d ", kipp->ki_pid);
	printf("%-16s ", kipp->ki_comm);
	printf("%5d ", kipp->ki_uid);
	printf("%5d ", kipp->ki_ruid);
	printf("%5d ", kipp->ki_svuid);
	printf("%5d ", kipp->ki_groups[0]);
	printf("%5d ", kipp->ki_rgid);
	printf("%5d ", kipp->ki_svgid);
	printf("%5s ", get_umask(procstat, kipp));
	printf("%s", kipp->ki_cr_flags & CRED_FLAG_CAPMODE ? "C" : "-");
	printf("     ");

	groups = NULL;
	/*
	 * We may have too many groups to fit in kinfo_proc's statically
	 * sized storage.  If that occurs, attempt to retrieve them using
	 * libprocstat.
	 */
	if (kipp->ki_cr_flags & KI_CRF_GRP_OVERFLOW)
		groups = procstat_getgroups(procstat, kipp, &ngroups);
	if (groups == NULL) {
		ngroups = kipp->ki_ngroups;
		groups = kipp->ki_groups;
	}
	for (i = 0; i < ngroups; i++)
		printf("%s%d", (i > 0) ? "," : "", groups[i]);
	if (groups != kipp->ki_groups)
		procstat_freegroups(procstat, groups);

	printf("\n");
}

static const char *
get_umask(struct procstat *procstat, struct kinfo_proc *kipp)
{
	u_short fd_cmask;
	static char umask[4];

	if (procstat_getumask(procstat, kipp, &fd_cmask) == 0) {
		snprintf(umask, 4, "%03o", fd_cmask);
		return (umask);
	} else {
		return ("-");
	}
}