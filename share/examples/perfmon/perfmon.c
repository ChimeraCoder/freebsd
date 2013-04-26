
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

#include <sys/types.h>
#include <sys/ioctl.h>

#include <machine/cpu.h>
#include <machine/perfmon.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

static int getnum(const char *, int, int);
static void usage(const char *) __dead2;

int
main(int argc, char **argv)
{
	int c, fd, num;
	int loops, i, sleeptime;
	char *cmd;
	struct pmc pmc;
	struct pmc_tstamp then, now;
	struct pmc_data value;
	quad_t *buf;
	double total;

	pmc.pmc_num = 0;
	pmc.pmc_event = 0;
	pmc.pmc_unit = 0;
	pmc.pmc_flags = 0;
	pmc.pmc_mask = 0;
	cmd = NULL;
	loops = 50;
	sleeptime = 0;

	while ((c = getopt(argc, argv, "s:l:uoeiU:m:c:")) != -1) {
		switch(c) {
		case 'u':
			pmc.pmc_flags |= PMCF_USR;
			break;
		case 'o':
			pmc.pmc_flags |= PMCF_OS;
			break;
		case 'e':
			pmc.pmc_flags |= PMCF_E;
			break;
		case 'i':
			pmc.pmc_flags |= PMCF_INV;
			break;
		case 'U':
			pmc.pmc_unit = getnum(optarg, 0, 256);
			break;
		case 'm':
			pmc.pmc_mask = getnum(optarg, 0, 256);
			break;
		case 'l':
			loops = getnum(optarg, 1, INT_MAX - 1);
			break;
		case 's':
			sleeptime = getnum(optarg, 0, INT_MAX - 1);
			break;
		case 'c':
			cmd = optarg;
			break;
		default:
			usage(argv[0]);
		}
	}

	if (argc - optind != 1)
		usage(argv[0]);

	pmc.pmc_event = getnum(argv[optind], 0, 255);

	buf = malloc((loops + 1) * sizeof *buf);
	if (!buf)
		err(1, "malloc(%lu)", (unsigned long)(loops +1) * sizeof *buf);

	fd = open(_PATH_PERFMON, O_RDWR, 0);
	if (fd < 0)
		err(1, "open: " _PATH_PERFMON);

	if (ioctl(fd, PMIOSETUP, &pmc) < 0)
		err(1, "ioctl(PMIOSETUP)");

	if (ioctl(fd, PMIOTSTAMP, &then) < 0)
		err(1, "ioctl(PMIOTSTAMP)");

	num = 0;
	if (ioctl(fd, PMIOSTART, &num) < 0)
		err(1, "ioctl(PMIOSTART)");

	value.pmcd_num = 0;
	for (i = 0; i < loops; i++) {
		if (ioctl(fd, PMIOSTOP, &num) < 0)
			err(1, "ioctl(PMIOSTOP)");
		if (ioctl(fd, PMIOREAD, &value) < 0)
			err(1, "ioctl(PMIOREAD)");
		buf[i] = value.pmcd_value;
		if (ioctl(fd, PMIORESET, &value.pmcd_num) < 0)
			err(1, "ioctl(PMIORESET)");
		if (ioctl(fd, PMIOSTART, &num) < 0)
			err(1, "ioctl(PMIOSTART)");
		if (sleeptime)
			sleep(sleeptime);
		if (cmd)
			system(cmd);
	}

	if (ioctl(fd, PMIOSTOP, &num) < 0)
		err(1, "ioctl(PMIOSTOP)");
	if (ioctl(fd, PMIOREAD, &value) < 0)
		err(1, "ioctl(PMIOREAD)");
	buf[i] = value.pmcd_value;
	if (ioctl(fd, PMIOTSTAMP, &now) < 0)
		err(1, "ioctl(PMIOTSTAMP)");

	total = 0;
	for (i = 1; i <= loops; i++) {
		printf("%d: %qd\n", i, buf[i]);
		total += buf[i];
	}
	printf("total: %f\nmean: %f\n", total, total / loops);

	printf("clocks (at %d-MHz): %qd\n", now.pmct_rate,
	       now.pmct_value - then.pmct_value);

	return 0;
}

static int
getnum(const char *buf, int min, int max)
{
	char *ep;
	long l;

	errno = 0;
	l = strtol(buf, &ep, 0);
	if (*buf && !*ep && !errno) {
		if (l < min || l > max) {
			errx(1, "%s: must be between %d and %d", 
			     buf, min, max);
		}
		return (int)l;
	} 

	errx(1, "%s: parameter must be an integer", buf);
}

static void
usage(const char *pname)
{
	fprintf(stderr, 
	"usage: %s [-eiou] [-c command] [-l nloops] [-m mask] [-s sleeptime]\n"
	"       [-U unit] counter\n",
		pname);
	exit(1);
}