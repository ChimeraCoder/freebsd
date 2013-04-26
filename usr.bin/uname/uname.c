
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

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#ifndef lint
static const char sccsid[] = "@(#)uname.c	8.2 (Berkeley) 5/4/95";
#endif

#include <sys/param.h>
#include <sys/sysctl.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define	MFLAG	0x01
#define	NFLAG	0x02
#define	PFLAG	0x04
#define	RFLAG	0x08
#define	SFLAG	0x10
#define	VFLAG	0x20
#define	IFLAG	0x40

typedef void (*get_t)(void);
static get_t get_ident, get_platform, get_hostname, get_arch,
    get_release, get_sysname, get_version;

static void native_ident(void);
static void native_platform(void);
static void native_hostname(void);
static void native_arch(void);
static void native_release(void);
static void native_sysname(void);
static void native_version(void);
static void print_uname(u_int);
static void setup_get(void);
static void usage(void);

static char *ident, *platform, *hostname, *arch, *release, *sysname, *version;
static int space;

int
main(int argc, char *argv[])
{
	u_int flags;
	int ch;

	setup_get();
	flags = 0;

	while ((ch = getopt(argc, argv, "aimnoprsv")) != -1)
		switch(ch) {
		case 'a':
			flags |= (MFLAG | NFLAG | RFLAG | SFLAG | VFLAG);
			break;
		case 'i':
			flags |= IFLAG;
			break;
		case 'm':
			flags |= MFLAG;
			break;
		case 'n':
			flags |= NFLAG;
			break;
		case 'p':
			flags |= PFLAG;
			break;
		case 'r':
			flags |= RFLAG;
			break;
		case 's':
		case 'o':
			flags |= SFLAG;
			break;
		case 'v':
			flags |= VFLAG;
			break;
		case '?':
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	if (argc)
		usage();

	if (!flags)
		flags |= SFLAG;

	print_uname(flags);
	exit(0);
}

#define	CHECK_ENV(opt,var)				\
do {							\
	if ((var = getenv("UNAME_" opt)) == NULL) {	\
		get_##var = native_##var;		\
	} else {					\
		get_##var = (get_t)NULL;		\
	}						\
} while (0)

static void
setup_get(void)
{
	CHECK_ENV("s", sysname);
	CHECK_ENV("n", hostname);
	CHECK_ENV("r", release);
	CHECK_ENV("v", version);
	CHECK_ENV("m", platform);
	CHECK_ENV("p", arch);
	CHECK_ENV("i", ident);
}

#define	PRINT_FLAG(flags,flag,var)		\
	if ((flags & flag) == flag) {		\
		if (space)			\
			printf(" ");		\
		else				\
			space++;		\
		if (get_##var != NULL)		\
			(*get_##var)();		\
		printf("%s", var);		\
	}

static void
print_uname(u_int flags)
{
	PRINT_FLAG(flags, SFLAG, sysname);
	PRINT_FLAG(flags, NFLAG, hostname);
	PRINT_FLAG(flags, RFLAG, release);
	PRINT_FLAG(flags, VFLAG, version);
	PRINT_FLAG(flags, MFLAG, platform);
	PRINT_FLAG(flags, PFLAG, arch);
	PRINT_FLAG(flags, IFLAG, ident);
	printf("\n");
}

#define	NATIVE_SYSCTL2_GET(var,mib0,mib1)	\
static void					\
native_##var(void)				\
{						\
	int mib[] = { (mib0), (mib1) };		\
	size_t len;				\
	static char buf[1024];			\
	char **varp = &(var);			\
						\
	len = sizeof buf;			\
	if (sysctl(mib, sizeof mib / sizeof mib[0],	\
	   &buf, &len, NULL, 0) == -1)		\
		err(1, "sysctl");

#define	NATIVE_SYSCTLNAME_GET(var,name)		\
static void					\
native_##var(void)				\
{						\
	size_t len;				\
	static char buf[1024];			\
	char **varp = &(var);			\
						\
	len = sizeof buf;			\
	if (sysctlbyname(name, &buf, &len, NULL,\
	    0) == -1)				\
		err(1, "sysctlbyname");

#define	NATIVE_SET				\
	*varp = buf;				\
	return;					\
}	struct __hack

#define	NATIVE_BUFFER	(buf)
#define	NATIVE_LENGTH	(len)

NATIVE_SYSCTL2_GET(sysname, CTL_KERN, KERN_OSTYPE) {
} NATIVE_SET;

NATIVE_SYSCTL2_GET(hostname, CTL_KERN, KERN_HOSTNAME) {
} NATIVE_SET;

NATIVE_SYSCTL2_GET(release, CTL_KERN, KERN_OSRELEASE) {
} NATIVE_SET;

NATIVE_SYSCTL2_GET(version, CTL_KERN, KERN_VERSION) {
	size_t n;
	char *p;

	p = NATIVE_BUFFER;
	n = NATIVE_LENGTH;
	for (; n--; ++p)
		if (*p == '\n' || *p == '\t')
			*p = ' ';
} NATIVE_SET;

NATIVE_SYSCTL2_GET(platform, CTL_HW, HW_MACHINE) {
} NATIVE_SET;

NATIVE_SYSCTL2_GET(arch, CTL_HW, HW_MACHINE_ARCH) {
} NATIVE_SET;

NATIVE_SYSCTLNAME_GET(ident, "kern.ident") {
} NATIVE_SET;

static void
usage(void)
{
	fprintf(stderr, "usage: uname [-aimnoprsv]\n");
	exit(1);
}