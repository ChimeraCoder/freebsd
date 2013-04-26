
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
 * cfi [-f device] op
 * (default device is /dev/cfi0).
 */
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/cfictl.h>

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *progname;
const char *dvname;

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-f device] op...\n", progname);
	fprintf(stderr, "where op's are:\n");
	fprintf(stderr, "fact\t\tread factory PR segment\n");
	fprintf(stderr, "oem\t\tread OEM segment\n");
	fprintf(stderr, "woem value\twrite OEM segment\n");
	fprintf(stderr, "plr\t\tread PLR\n");
	fprintf(stderr, "wplr\t\twrite PLR\n");
	exit(-1);
}

static int
getfd(int mode)
{
	int fd = open(dvname, mode, 0);
	if (fd < 0)
		err(-1, "open");
	return fd;
}

static uint64_t
getfactorypr(void)
{
	uint64_t v;
	int fd = getfd(O_RDONLY);
	if (ioctl(fd, CFIOCGFACTORYPR, &v) < 0)
		err(-1, "ioctl(CFIOCGFACTORYPR)");
	close(fd);
	return v;
}

static uint64_t
getoempr(void)
{
	uint64_t v;
	int fd = getfd(O_RDONLY);
	if (ioctl(fd, CFIOCGOEMPR, &v) < 0)
		err(-1, "ioctl(CFIOCGOEMPR)");
	close(fd);
	return v;
}

static void
setoempr(uint64_t v)
{
	int fd = getfd(O_WRONLY);
	if (ioctl(fd, CFIOCSOEMPR, &v) < 0)
		err(-1, "ioctl(CFIOCGOEMPR)");
	close(fd);
}

static uint32_t
getplr(void)
{
	uint32_t plr;
	int fd = getfd(O_RDONLY);
	if (ioctl(fd, CFIOCGPLR, &plr) < 0)
		err(-1, "ioctl(CFIOCGPLR)");
	close(fd);
	return plr;
}

static void
setplr(void)
{
	int fd = getfd(O_WRONLY);
	if (ioctl(fd, CFIOCSPLR, 0) < 0)
		err(-1, "ioctl(CFIOCPLR)");
	close(fd);
}

int
main(int argc, char *argv[])
{
	dvname = getenv("CFI");
	if (dvname == NULL)
		dvname = "/dev/cfi0";
	progname = argv[0];
	if (argc > 1) {
		if (strcmp(argv[1], "-f") == 0) {
			if (argc < 2)
				errx(1, "missing device name for -f option");
			dvname = argv[2];
			argc -= 2, argv += 2;
		} else if (strcmp(argv[1], "-?") == 0)
			usage();
	}
	for (; argc > 1; argc--, argv++) {
		if (strcasecmp(argv[1], "fact") == 0) {
			printf("0x%llx\n", (unsigned long long) getfactorypr());
		} else if (strcasecmp(argv[1], "oem") == 0) {
			printf("0x%llx\n", (unsigned long long) getoempr());
		} else if (strcasecmp(argv[1], "woem") == 0) {
			if (argc < 2)
				errx(1, "missing value for woem");
			setoempr((uint64_t) strtoull(argv[2], NULL, 0));
			argc--, argv++;
		} else if (strcasecmp(argv[1], "plr") == 0) {
			printf("0x%x\n", getplr());
		} else if (strcasecmp(argv[1], "wplr") == 0) {
			setplr();
		} else
			usage();
	}
}