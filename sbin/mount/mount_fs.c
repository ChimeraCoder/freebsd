
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

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1992, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)mount_fs.c	8.6 (Berkeley) 4/26/95";
#endif
static const char rcsid[] =
	"$FreeBSD$";
#endif /* not lint */

#include <sys/param.h>
#include <sys/mount.h>

#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"
#include "mntopts.h"

static struct mntopt mopts[] = {
	MOPT_STDOPTS,
	MOPT_END
};

static void
usage(void)
{
	(void)fprintf(stderr,
		"usage: mount [-t fstype] [-o options] target_fs mount_point\n");
	exit(1);
}

int
mount_fs(const char *vfstype, int argc, char *argv[])
{
	struct iovec *iov;
	int iovlen;
	int mntflags = 0;
	int ch;
	char *dev, *dir, mntpath[MAXPATHLEN];
	char fstype[32];
	char errmsg[255];
	char *p, *val;

	strlcpy(fstype, vfstype, sizeof(fstype));
	memset(errmsg, 0, sizeof(errmsg));

	getmnt_silent = 1;
	iov = NULL;
	iovlen = 0;

	optind = optreset = 1;		/* Reset for parse of new argv. */
	while ((ch = getopt(argc, argv, "o:")) != -1) {
		switch(ch) {
		case 'o':
			getmntopts(optarg, mopts, &mntflags, 0);
			p = strchr(optarg, '=');
			val = NULL;
			if (p != NULL) {
				*p = '\0';
				val = p + 1;
			}
			build_iovec(&iov, &iovlen, optarg, val, (size_t)-1);
			break;
		case '?':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;
	if (argc != 2)
		usage();

	dev = argv[0];
	dir = argv[1];

	if (checkpath(dir, mntpath) != 0) {
		warn("%s", mntpath);
		return (1);
	}
	(void)rmslashes(dev, dev);

	build_iovec(&iov, &iovlen, "fstype", fstype, (size_t)-1);
	build_iovec(&iov, &iovlen, "fspath", mntpath, (size_t)-1);
	build_iovec(&iov, &iovlen, "from", dev, (size_t)-1);
	build_iovec(&iov, &iovlen, "errmsg", errmsg, sizeof(errmsg));

	if (nmount(iov, iovlen, mntflags) == -1) {
		if (*errmsg != '\0')
			warn("%s: %s", dev, errmsg);
		else
			warn("%s", dev);
		return (1);
	}
	return (0);
}