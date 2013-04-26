
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

/*
 * Simple Atheros-specific tool to inspect and monitor software queue
 * and aggregate statistics.
 *
 *	athaggrstats [-i interface] [-bz] [-l] [-o fmtstring] [interval]
 *
 * (default interface is ath0).  If interval is specified a rolling output
 * a la netstat -i is displayed every interval seconds.  The format of
 * the rolling display can be controlled a la ps.  The -l option will
 * print a list of all possible statistics for use with the -o option.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <err.h>

#include "athaggrstats.h"

static struct {
	const char *tag;
	const char *fmt;
} tags[] = {
  { "default",
    "singlepkt,nonbawpkt,aggrpkt,bawclosedpkt,lhsinglepkt,schednopkt,rtsaggrlimit"
  },
};

static const char *
getfmt(const char *tag)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	int i;
	for (i = 0; i < N(tags); i++)
		if (strcasecmp(tags[i].tag, tag) == 0)
			return tags[i].fmt;
	return tag;
#undef N
}

static int signalled;

static void
catchalarm(int signo __unused)
{
	signalled = 1;
}

int
main(int argc, char *argv[])
{
	struct athaggrstatfoo *wf;
	const char *ifname;
	int c, banner = 1;

	ifname = getenv("ATH");
	if (ifname == NULL)
		ifname = "ath0";
	wf = athaggrstats_new(ifname, getfmt("default"));
	while ((c = getopt(argc, argv, "bi:lo:z")) != -1) {
		switch (c) {
		case 'b':
			banner = 0;
			break;
		case 'i':
			wf->setifname(wf, optarg);
			break;
		case 'l':
			wf->print_fields(wf, stdout);
			return 0;
		case 'o':
			wf->setfmt(wf, getfmt(optarg));
			break;
		case 'z':
			wf->zerostats(wf);
			break;
		default:
			errx(-1, "usage: %s [-a] [-i ifname] [-l] [-o fmt] [-z] [interval]\n", argv[0]);
			/*NOTREACHED*/
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		u_long interval = strtoul(argv[0], NULL, 0);
		int line, omask;

		if (interval < 1)
			interval = 1;
		signal(SIGALRM, catchalarm);
		signalled = 0;
		alarm(interval);
	banner:
		if (banner)
			wf->print_header(wf, stdout);
		line = 0;
	loop:
		if (line != 0) {
			wf->collect_cur(wf);
			wf->print_current(wf, stdout);
			wf->update_tot(wf);
		} else {
			wf->collect_tot(wf);
			wf->print_total(wf, stdout);
		}
		fflush(stdout);
		omask = sigblock(sigmask(SIGALRM));
		if (!signalled)
			sigpause(0);
		sigsetmask(omask);
		signalled = 0;
		alarm(interval);
		line++;
		if (line == 21)		/* XXX tty line count */
			goto banner;
		else
			goto loop;
		/*NOTREACHED*/
	} else {
		wf->collect_tot(wf);
		wf->print_verbose(wf, stdout);
	}
	return 0;
}