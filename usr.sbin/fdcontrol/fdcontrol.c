
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/fdcio.h>
#include <sys/file.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "fdutil.h"


static	int format, verbose, show = 1, showfmt;
static	char *fmtstring;

static void showdev(enum fd_drivetype, const char *);
static void usage(void);

static void
usage(void)
{
	errx(EX_USAGE,
	     "usage: fdcontrol [-F] [-d dbg] [-f fmt] [-s fmtstr] [-v] device");
}

void
showdev(enum fd_drivetype type, const char *fname)
{
	const char *name, *descr;

	getname(type, &name, &descr);
	if (verbose)
		printf("%s: %s drive (%s)\n", fname, name, descr);
	else
		printf("%s\n", name);
}

int
main(int argc, char **argv)
{
	enum fd_drivetype type;
	struct fd_type ft, newft, *fdtp;
	const char *name, *descr;
	int fd, i, autofmt;

	autofmt = 0;
	while((i = getopt(argc, argv, "aFf:s:v")) != -1)
		switch(i) {

		case 'a':
			autofmt = 1;
		case 'F':
			showfmt = 1;
			show = 0;
			break;

		case 'f':
			if (!strcmp(optarg, "auto")) {
				format = -1;
			} else if (getnum(optarg, &format)) {
				fprintf(stderr,
			"Bad argument %s to -f option; must be numeric\n",
					optarg);
				usage();
			}
			show = 0;
			break;

		case 's':
			fmtstring = optarg;
			show = 0;
			break;

		case 'v':
			verbose++;
			break;

		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	if(argc != 1)
		usage();

	if((fd = open(argv[0], O_RDONLY | O_NONBLOCK)) < 0)
		err(EX_UNAVAILABLE, "open(%s)", argv[0]);

	if (ioctl(fd, FD_GDTYPE, &type) == -1)
		err(EX_OSERR, "ioctl(FD_GDTYPE)");
	if (ioctl(fd, FD_GTYPE, &ft) == -1)
		err(EX_OSERR, "ioctl(FD_GTYPE)");

	if (show) {
		showdev(type, argv[0]);
		return (0);
	}

	if (autofmt) {
		memset(&newft, 0, sizeof newft);
		ft = newft;
	}

	if (format) {
		getname(type, &name, &descr);
		fdtp = get_fmt(format, type);
		if (fdtp == 0)
			errx(EX_USAGE,
			    "unknown format %d KB for drive type %s",
			    format, name);
		ft = *fdtp;
	}

	if (fmtstring) {
		parse_fmt(fmtstring, type, ft, &newft);
		ft = newft;
	}

	if (showfmt) {
		if (verbose) {
			const char *s;

			printf("%s: %d KB media type\n", argv[0],
			    (128 << ft.secsize) * ft.size / 1024);
			printf("\tFormat:\t\t");
			print_fmt(ft);
			if (ft.datalen != 0xff &&
			    ft.datalen != (128 << ft.secsize))
				printf("\tData length:\t%d\n", ft.datalen);
			printf("\tSector size:\t%d\n", 128 << ft.secsize);
			printf("\tSectors/track:\t%d\n", ft.sectrac);
			printf("\tHeads/cylinder:\t%d\n", ft.heads);
			printf("\tCylinders/disk:\t%d\n", ft.tracks);
			switch (ft.trans) {
			case 0: printf("\tTransfer rate:\t500 kbps\n"); break;
			case 1: printf("\tTransfer rate:\t300 kbps\n"); break;
			case 2: printf("\tTransfer rate:\t250 kbps\n"); break;
			case 3: printf("\tTransfer rate:\t1 Mbps\n"); break;
			}
			printf("\tSector gap:\t%d\n", ft.gap);
			printf("\tFormat gap:\t%d\n", ft.f_gap);
			printf("\tInterleave:\t%d\n", ft.f_inter);
			printf("\tSide offset:\t%d\n", ft.offset_side2);
			printf("\tFlags\t\t<");
			s = "";
			if (ft.flags & FL_MFM) {
				printf("%sMFM", s);
				s = ",";
			}
			if (ft.flags & FL_2STEP) {
				printf("%s2STEP", s);
				s = ",";
			}
			if (ft.flags & FL_PERPND) {
				printf("%sPERPENDICULAR", s);
				s = ",";
			}
			if (ft.flags & FL_AUTO) {
				printf("%sAUTO", s);
				s = ",";
			}
			printf(">\n");
		} else {
			print_fmt(ft);
		}
		return (0);
	}

	if (format || fmtstring) {
		if (ioctl(fd, FD_STYPE, &ft) == -1)
			err(EX_OSERR, "ioctl(FD_STYPE)");
		return (0);
	}

	return 0;
}