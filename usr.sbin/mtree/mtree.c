
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

#if 0
#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1989, 1990, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)mtree.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <fts.h>
#include <stdio.h>
#include <unistd.h>
#include "mtree.h"
#include "extern.h"

int ftsoptions = FTS_PHYSICAL;
int dflag, eflag, iflag, nflag, qflag, rflag, sflag, uflag, wflag;
static int cflag, Uflag;
u_int keys;
char fullpath[MAXPATHLEN];

static void usage(void);

int
main(int argc, char *argv[])
{
	int ch;
	char *dir, *p;
	int status;
	FILE *spec1, *spec2;

	dir = NULL;
	keys = KEYDEFAULT;
	init_excludes();
	spec1 = stdin;
	spec2 = NULL;

	while ((ch = getopt(argc, argv, "cdef:iK:k:LnPp:qrs:UuwxX:")) != -1)
		switch((char)ch) {
		case 'c':
			cflag = 1;
			break;
		case 'd':
			dflag = 1;
			break;
		case 'e':
			eflag = 1;
			break;
		case 'f':
			if (spec1 == stdin) {
				spec1 = fopen(optarg, "r");
				if (spec1 == NULL)
					err(1, "%s", optarg);
			} else if (spec2 == NULL) {
				spec2 = fopen(optarg, "r");
				if (spec2 == NULL)
					err(1, "%s", optarg);
			} else
				usage();
			break;
		case 'i':
			iflag = 1;
			break;
		case 'K':
			while ((p = strsep(&optarg, " \t,")) != NULL)
				if (*p != '\0')
					keys |= parsekey(p, NULL);
			break;
		case 'k':
			keys = F_TYPE;
			while ((p = strsep(&optarg, " \t,")) != NULL)
				if (*p != '\0')
					keys |= parsekey(p, NULL);
			break;
		case 'L':
			ftsoptions &= ~FTS_PHYSICAL;
			ftsoptions |= FTS_LOGICAL;
			break;
		case 'n':
			nflag = 1;
			break;
		case 'P':
			ftsoptions &= ~FTS_LOGICAL;
			ftsoptions |= FTS_PHYSICAL;
			break;
		case 'p':
			dir = optarg;
			break;
		case 'q':
			qflag = 1;
			break;
		case 'r':
			rflag = 1;
			break;
		case 's':
			sflag = 1;
			crc_total = ~strtoul(optarg, &p, 0);
			if (*p)
				errx(1, "illegal seed value -- %s", optarg);
			break;
		case 'U':
			Uflag = 1;
			uflag = 1;
			break;
		case 'u':
			uflag = 1;
			break;
		case 'w':
			wflag = 1;
			break;
		case 'x':
			ftsoptions |= FTS_XDEV;
			break;
		case 'X':
			read_excludes_file(optarg);
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc)
		usage();

	if (dir && chdir(dir))
		err(1, "%s", dir);

	if ((cflag || sflag) && !getcwd(fullpath, sizeof(fullpath)))
		errx(1, "%s", fullpath);

	if (cflag) {
		cwalk();
		exit(0);
	}
	if (spec2 != NULL)
		status = mtree_specspec(spec1, spec2);
	else
		status = mtree_verifyspec(spec1);
	if (Uflag & (status == MISMATCHEXIT))
		status = 0;
	exit(status);
}

static void
usage(void)
{
	(void)fprintf(stderr,
"usage: mtree [-LPUcdeinqruxw] [-f spec] [-f spec] [-K key] [-k key] [-p path] [-s seed]\n"
"\t[-X excludes]\n");
	exit(1);
}