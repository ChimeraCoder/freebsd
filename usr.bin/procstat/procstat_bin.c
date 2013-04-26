
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
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "procstat.h"

void
procstat_bin(struct procstat *prstat, struct kinfo_proc *kipp)
{
	int osrel;
	static char pathname[PATH_MAX];

	if (!hflag)
		printf("%5s %-16s %8s %s\n", "PID", "COMM", "OSREL", "PATH");

	if (procstat_getpathname(prstat, kipp, pathname, sizeof(pathname)) != 0)
		return;
	if (strlen(pathname) == 0)
		strcpy(pathname, "-");
	if (procstat_getosrel(prstat, kipp, &osrel) != 0)
		return;

	printf("%5d ", kipp->ki_pid);
	printf("%-16s ", kipp->ki_comm);
	printf("%8d ", osrel);
	printf("%s\n", pathname);
}