
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
#include <stdlib.h>
#include <string.h>

#include "procstat.h"

static void
do_args(struct procstat *procstat, struct kinfo_proc *kipp, int env)
{
	int i;
	char **args;

	if (!hflag) {
		printf("%5s %-16s %-53s\n", "PID", "COMM",
		    env ? "ENVIRONMENT" : "ARGS");
	}

	args = env ? procstat_getenvv(procstat, kipp, 0) :
	    procstat_getargv(procstat, kipp, 0);

	printf("%5d %-16s", kipp->ki_pid, kipp->ki_comm);

	if (args == NULL) {
		printf(" -\n");
		return;
	}

	for (i = 0; args[i] != NULL; i++)
		printf(" %s", args[i]);
	printf("\n");
}

void
procstat_args(struct procstat *procstat, struct kinfo_proc *kipp)
{
	do_args(procstat, kipp, 0);
}

void
procstat_env(struct procstat *procstat, struct kinfo_proc *kipp)
{
	do_args(procstat, kipp, 1);
}