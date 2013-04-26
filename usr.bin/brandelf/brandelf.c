
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

#include <sys/types.h>
#include <sys/elf_common.h>
#include <sys/errno.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int elftype(const char *);
static const char *iselftype(int);
static void printelftypes(void);
static void usage(void);

struct ELFtypes {
	const char *str;
	int value;
};
/* XXX - any more types? */
static struct ELFtypes elftypes[] = {
	{ "FreeBSD",	ELFOSABI_FREEBSD },
	{ "Linux",	ELFOSABI_LINUX },
	{ "Solaris",	ELFOSABI_SOLARIS },
	{ "SVR4",	ELFOSABI_SYSV }
};

int
main(int argc, char **argv)
{

	const char *strtype = "FreeBSD";
	int type = ELFOSABI_FREEBSD;
	int retval = 0;
	int ch, change = 0, force = 0, listed = 0;

	while ((ch = getopt(argc, argv, "f:lt:v")) != -1)
		switch (ch) {
		case 'f':
			if (change)
				errx(1, "f option incompatible with t option");
			force = 1;
			type = atoi(optarg);
			if (errno == ERANGE || type < 0 || type > 255) {
				warnx("invalid argument to option f: %s",
				    optarg);
				usage();
			}
			break;
		case 'l':
			printelftypes();
			listed = 1;
			break;
		case 'v':
			/* does nothing */
			break;
		case 't':
			if (force)
				errx(1, "t option incompatible with f option");
			change = 1;
			strtype = optarg;
			break;
		default:
			usage();
	}
	argc -= optind;
	argv += optind;
	if (!argc) {
		if (listed)
			exit(0);
		else {
			warnx("no file(s) specified");
			usage();
		}
	}

	if (!force && (type = elftype(strtype)) == -1) {
		warnx("invalid ELF type '%s'", strtype);
		printelftypes();
		usage();
	}

	while (argc) {
		int fd;
		char buffer[EI_NIDENT];

		if ((fd = open(argv[0], change || force ? O_RDWR : O_RDONLY, 0)) < 0) {
			warn("error opening file %s", argv[0]);
			retval = 1;
			goto fail;
		}
		if (read(fd, buffer, EI_NIDENT) < EI_NIDENT) {
			warnx("file '%s' too short", argv[0]);
			retval = 1;
			goto fail;
		}
		if (buffer[0] != ELFMAG0 || buffer[1] != ELFMAG1 ||
		    buffer[2] != ELFMAG2 || buffer[3] != ELFMAG3) {
			warnx("file '%s' is not ELF format", argv[0]);
			retval = 1;
			goto fail;
		}
		if (!change && !force) {
			fprintf(stdout,
				"File '%s' is of brand '%s' (%u).\n",
				argv[0], iselftype(buffer[EI_OSABI]),
				buffer[EI_OSABI]);
			if (!iselftype(type)) {
				warnx("ELF ABI Brand '%u' is unknown",
				      type);
				printelftypes();
			}
		}
		else {
			buffer[EI_OSABI] = type;
			lseek(fd, 0, SEEK_SET);
			if (write(fd, buffer, EI_NIDENT) != EI_NIDENT) {
				warn("error writing %s %d", argv[0], fd);
				retval = 1;
				goto fail;
			}
		}
fail:
		close(fd);
		argc--;
		argv++;
	}

	return retval;
}

static void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: brandelf [-lv] [-f ELF_ABI_number] [-t string] file ...\n");
	exit(1);
}

static const char *
iselftype(int etype)
{
	size_t elfwalk;

	for (elfwalk = 0;
	     elfwalk < sizeof(elftypes)/sizeof(elftypes[0]);
	     elfwalk++)
		if (etype == elftypes[elfwalk].value)
			return elftypes[elfwalk].str;
	return 0;
}

static int
elftype(const char *elfstrtype)
{
	size_t elfwalk;

	for (elfwalk = 0;
	     elfwalk < sizeof(elftypes)/sizeof(elftypes[0]);
	     elfwalk++)
		if (strcasecmp(elfstrtype, elftypes[elfwalk].str) == 0)
			return elftypes[elfwalk].value;
	return -1;
}

static void
printelftypes(void)
{
	size_t elfwalk;

	fprintf(stderr, "known ELF types are: ");
	for (elfwalk = 0;
	     elfwalk < sizeof(elftypes)/sizeof(elftypes[0]);
	     elfwalk++)
		fprintf(stderr, "%s(%u) ", elftypes[elfwalk].str, 
			elftypes[elfwalk].value);
	fprintf(stderr, "\n");
}