
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
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "From: @(#)swapon.c	8.1 (Berkeley) 6/5/93";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/disk.h>
#include <sys/sysctl.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

static int	verbose;

static void
usage(void)
{
	fprintf(stderr, "%s\n%s\n%s\n",
	    "usage: dumpon [-v] special_file",
	    "       dumpon [-v] off",
	    "       dumpon [-v] -l");
	exit(EX_USAGE);
}

static void
check_size(int fd, const char *fn)
{
	int name[] = { CTL_HW, HW_PHYSMEM };
	size_t namelen = sizeof(name) / sizeof(*name);
	unsigned long physmem;
	size_t len;
	off_t mediasize;
	int minidump;

	len = sizeof(minidump);
	if (sysctlbyname("debug.minidump", &minidump, &len, NULL, 0) == 0 &&
	    minidump == 1)
		return;
	len = sizeof(physmem);
	if (sysctl(name, namelen, &physmem, &len, NULL, 0) != 0)
		err(EX_OSERR, "can't get memory size");
	if (ioctl(fd, DIOCGMEDIASIZE, &mediasize) != 0)
		err(EX_OSERR, "%s: can't get size", fn);
	if ((uintmax_t)mediasize < (uintmax_t)physmem) {
		if (verbose)
			printf("%s is smaller than physical memory\n", fn);
		exit(EX_IOERR);
	}
}

static void
listdumpdev(void)
{
	char dumpdev[PATH_MAX];
	size_t len;
	const char *sysctlname = "kern.shutdown.dumpdevname";

	len = sizeof(dumpdev);
	if (sysctlbyname(sysctlname, &dumpdev, &len, NULL, 0) != 0) {
		if (errno == ENOMEM) {
			err(EX_OSERR, "Kernel returned too large of a buffer for '%s'\n",
				sysctlname);
		} else {
			err(EX_OSERR, "Sysctl get '%s'\n", sysctlname);
		}
	}
	if (verbose) {
		printf("kernel dumps on ");
	}
	if (strlen(dumpdev) == 0) {
		printf("%s\n", _PATH_DEVNULL);
	} else {
		printf("%s\n", dumpdev);
	}
}

int
main(int argc, char *argv[])
{
	int ch;
	int i, fd;
	u_int u;
	int do_listdumpdev = 0;

	while ((ch = getopt(argc, argv, "lv")) != -1)
		switch((char)ch) {
		case 'l':
			do_listdumpdev = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	if (do_listdumpdev) {
		listdumpdev();
		exit(EX_OK);
	}

	if (argc != 1)
		usage();

	if (strcmp(argv[0], "off") != 0) {
		fd = open(argv[0], O_RDONLY);
		if (fd < 0)
			err(EX_OSFILE, "%s", argv[0]);
		check_size(fd, argv[0]);
		u = 0;
		i = ioctl(fd, DIOCSKERNELDUMP, &u);
		u = 1;
		i = ioctl(fd, DIOCSKERNELDUMP, &u);
		if (i == 0 && verbose)
			printf("kernel dumps on %s\n", argv[0]);
	} else {
		fd = open(_PATH_DEVNULL, O_RDONLY);
		if (fd < 0)
			err(EX_OSFILE, "%s", _PATH_DEVNULL);
		u = 0;
		i = ioctl(fd, DIOCSKERNELDUMP, &u);
		if (i == 0 && verbose)
			printf("kernel dumps disabled\n");
	}
	if (i < 0)
		err(EX_OSERR, "ioctl(DIOCSKERNELDUMP)");

	exit (0);
}