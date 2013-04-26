
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
#include <sys/time.h>
#include <sys/resourcevar.h>
#include <sys/sysctl.h>
#include <sys/user.h>

#include <err.h>
#include <errno.h>
#include <libprocstat.h>
#include <libutil.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procstat.h"

static struct {
	const char *name;
	const char *suffix;
} rlimit_param[13] = {
	{"cputime",          "sec"},
	{"filesize",         "B  "},
	{"datasize",         "B  "},
	{"stacksize",        "B  "},
	{"coredumpsize",     "B  "},
	{"memoryuse",        "B  "},
	{"memorylocked",     "B  "},
	{"maxprocesses",     "   "},
	{"openfiles",        "   "},
	{"sbsize",           "B  "},
	{"vmemoryuse",       "B  "},
	{"pseudo-terminals", "   "},
	{"swapuse",          "B  "},
};

#if RLIM_NLIMITS > 13
#error "Resource limits have grown. Add new entries to rlimit_param[]."
#endif

static const char *
humanize_rlimit(int indx, rlim_t limit)
{
	static char buf[14];
	int scale;

	if (limit == RLIM_INFINITY)
		return ("infinity     ");

	scale = humanize_number(buf, sizeof(buf) - 1, (int64_t)limit,
	    rlimit_param[indx].suffix, HN_AUTOSCALE | HN_GETSCALE, HN_DECIMAL);
	(void)humanize_number(buf, sizeof(buf) - 1, (int64_t)limit,
	    rlimit_param[indx].suffix, HN_AUTOSCALE, HN_DECIMAL);
	/* Pad with one space if there is no suffix prefix. */
	if (scale == 0)
		sprintf(buf + strlen(buf), " ");
	return (buf);
}

void
procstat_rlimit(struct procstat *prstat, struct kinfo_proc *kipp)
{
	struct rlimit rlimit;
	int i;

	if (!hflag) {
		printf("%5s %-16s %-16s %16s %16s\n",
		    "PID", "COMM", "RLIMIT", "SOFT     ", "HARD     ");
	}
	for (i = 0; i < RLIM_NLIMITS; i++) {
		if (procstat_getrlimit(prstat, kipp, i, &rlimit) == -1)
			return;
		printf("%5d %-16s %-16s ", kipp->ki_pid, kipp->ki_comm,
		    rlimit_param[i].name);
		printf("%16s ", humanize_rlimit(i, rlimit.rlim_cur));
		printf("%16s\n", humanize_rlimit(i, rlimit.rlim_max));
	}
}