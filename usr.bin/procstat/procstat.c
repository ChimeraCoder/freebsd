
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
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include "procstat.h"

static int aflag, bflag, cflag, eflag, fflag, iflag, jflag, kflag, lflag, sflag;
static int tflag, vflag, xflag;
int	hflag, nflag, Cflag;

static void
usage(void)
{

	fprintf(stderr, "usage: procstat [-h] [-C] [-M core] [-N system] "
	    "[-w interval] \n");
	fprintf(stderr, "                [-b | -c | -e | -f | -i | -j | -k | "
	    "-l | -s | -t | -v | -x] [-a | pid | core ...]\n");
	exit(EX_USAGE);
}

static void
procstat(struct procstat *prstat, struct kinfo_proc *kipp)
{

	if (bflag)
		procstat_bin(prstat, kipp);
	else if (cflag)
		procstat_args(prstat, kipp);
	else if (eflag)
		procstat_env(prstat, kipp);
	else if (fflag)
		procstat_files(prstat, kipp);
	else if (iflag)
		procstat_sigs(prstat, kipp);
	else if (jflag)
		procstat_threads_sigs(prstat, kipp);
	else if (kflag)
		procstat_kstack(prstat, kipp, kflag);
	else if (lflag)
		procstat_rlimit(prstat, kipp);
	else if (sflag)
		procstat_cred(prstat, kipp);
	else if (tflag)
		procstat_threads(prstat, kipp);
	else if (vflag)
		procstat_vm(prstat, kipp);
	else if (xflag)
		procstat_auxv(prstat, kipp);
	else
		procstat_basic(kipp);
}

/*
 * Sort processes first by pid and then tid.
 */
static int
kinfo_proc_compare(const void *a, const void *b)
{
	int i;

	i = ((const struct kinfo_proc *)a)->ki_pid -
	    ((const struct kinfo_proc *)b)->ki_pid;
	if (i != 0)
		return (i);
	i = ((const struct kinfo_proc *)a)->ki_tid -
	    ((const struct kinfo_proc *)b)->ki_tid;
	return (i);
}

void
kinfo_proc_sort(struct kinfo_proc *kipp, int count)
{

	qsort(kipp, count, sizeof(*kipp), kinfo_proc_compare);
}

int
main(int argc, char *argv[])
{
	int ch, interval, tmp;
	int i;
	struct kinfo_proc *p;
	struct procstat *prstat, *cprstat;
	long l;
	pid_t pid;
	char *dummy;
	char *nlistf, *memf;
	int cnt;

	interval = 0;
	memf = nlistf = NULL;
	while ((ch = getopt(argc, argv, "CN:M:abcefijklhstvw:x")) != -1) {
		switch (ch) {
		case 'C':
			Cflag++;
			break;

		case 'M':
			memf = optarg;
			break;
		case 'N':
			nlistf = optarg;
			break;
		case 'a':
			aflag++;
			break;

		case 'b':
			bflag++;
			break;

		case 'c':
			cflag++;
			break;

		case 'e':
			eflag++;
			break;

		case 'f':
			fflag++;
			break;

		case 'i':
			iflag++;
			break;

		case 'j':
			jflag++;
			break;

		case 'k':
			kflag++;
			break;

		case 'l':
			lflag++;
			break;

		case 'n':
			nflag++;
			break;

		case 'h':
			hflag++;
			break;

		case 's':
			sflag++;
			break;

		case 't':
			tflag++;
			break;

		case 'v':
			vflag++;
			break;

		case 'w':
			l = strtol(optarg, &dummy, 10);
			if (*dummy != '\0')
				usage();
			if (l < 1 || l > INT_MAX)
				usage();
			interval = l;
			break;

		case 'x':
			xflag++;
			break;

		case '?':
		default:
			usage();
		}

	}
	argc -= optind;
	argv += optind;

	/* We require that either 0 or 1 mode flags be set. */
	tmp = bflag + cflag + eflag + fflag + iflag + jflag + (kflag ? 1 : 0) +
	    lflag + sflag + tflag + vflag + xflag;
	if (!(tmp == 0 || tmp == 1))
		usage();

	/* We allow -k to be specified up to twice, but not more. */
	if (kflag > 2)
		usage();

	/* Must specify either the -a flag or a list of pids. */
	if (!(aflag == 1 && argc == 0) && !(aflag == 0 && argc > 0))
		usage();

	/* Only allow -C with -f. */
	if (Cflag && !fflag)
		usage();

	if (memf != NULL)
		prstat = procstat_open_kvm(nlistf, memf);
	else
		prstat = procstat_open_sysctl();
	if (prstat == NULL)
		errx(1, "procstat_open()");
	do {
		if (aflag) {
			p = procstat_getprocs(prstat, KERN_PROC_PROC, 0, &cnt);
			if (p == NULL)
				errx(1, "procstat_getprocs()");
			kinfo_proc_sort(p, cnt);
			for (i = 0; i < cnt; i++) {
				procstat(prstat, &p[i]);

				/* Suppress header after first process. */
				hflag = 1;
			}
			procstat_freeprocs(prstat, p);
		}
		for (i = 0; i < argc; i++) {
			l = strtol(argv[i], &dummy, 10);
			if (*dummy == '\0') {
				if (l < 0)
					usage();
				pid = l;

				p = procstat_getprocs(prstat, KERN_PROC_PID, pid, &cnt);
				if (p == NULL)
					errx(1, "procstat_getprocs()");
				if (cnt != 0)
					procstat(prstat, p);
				procstat_freeprocs(prstat, p);
			} else {
				cprstat = procstat_open_core(argv[i]);
				if (cprstat == NULL) {
					warnx("procstat_open()");
					continue;
				}
				p = procstat_getprocs(cprstat, KERN_PROC_PID,
				    -1, &cnt);
				if (p == NULL)
					errx(1, "procstat_getprocs()");
				if (cnt != 0)
					procstat(cprstat, p);
				procstat_freeprocs(cprstat, p);
				procstat_close(cprstat);
			}
			/* Suppress header after first process. */
			hflag = 1;
		}
		if (interval)
			sleep(interval);
	} while (interval);
	procstat_close(prstat);
	exit(0);
}