
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <sys/digiio.h>
#include <sys/ioctl.h>
#include <unistd.h>

enum aflag { NO_AFLAG, ALTPIN_DISABLE, ALTPIN_ENABLE, ALTPIN_QUERY };

static int
usage(const char *prog)
{
	fprintf(stderr, "usage: %s -a disable|enable|query device\n", prog);
	fprintf(stderr, "       %s [-d debug] [-ir] ctrl-device...\n", prog);
	return (EX_USAGE);
}

int
main(int argc, char **argv)
{
	char namedata[256], *name = namedata;
	const char *prog;
	enum digi_model model;
	int altpin, ch, debug, fd, i, res;
	int dflag, iflag, rflag;
	enum aflag aflag;

	if ((prog = strrchr(argv[0], '/')) == NULL)
		prog = argv[0];
	else
		prog++;

	aflag = NO_AFLAG;
	dflag = iflag = rflag = 0;
	while ((ch = getopt(argc, argv, "a:d:ir")) != -1)
		switch (ch) {
		case 'a':
			if (strcasecmp(optarg, "disable") == 0)
				aflag = ALTPIN_DISABLE;
			else if (strcasecmp(optarg, "enable") == 0)
				aflag = ALTPIN_ENABLE;
			else if (strcasecmp(optarg, "query") == 0)
				aflag = ALTPIN_QUERY;
			else
				return (usage(prog));
			break;

		case 'd':
			dflag = 1;
			debug = atoi(optarg);
			break;

		case 'i':
			iflag = 1;
			break;

		case 'r':
			rflag = 1;
			break;

		default:
			return (usage(prog));
		}

	if ((dflag || iflag || rflag) && aflag != NO_AFLAG)
		return (usage(prog));

	if (argc <= optind)
		return (usage(prog));

	altpin = (aflag == ALTPIN_ENABLE);
	res = 0;

	for (i = optind; i < argc; i++) {
		if ((fd = open(argv[i], O_RDONLY)) == -1) {
			fprintf(stderr, "%s: %s: open: %s\n", prog, argv[i],
			    strerror(errno));
			res++;
			continue;
		}

		switch (aflag) {
		case NO_AFLAG:
			break;

		case ALTPIN_ENABLE:
		case ALTPIN_DISABLE:
			if (ioctl(fd, DIGIIO_SETALTPIN, &altpin) != 0) {
				fprintf(stderr, "%s: %s: set altpin: %s\n",
				    prog, argv[i], strerror(errno));
				res++;
			}
			break;

		case ALTPIN_QUERY:
			if (ioctl(fd, DIGIIO_GETALTPIN, &altpin) != 0) {
				fprintf(stderr, "%s: %s: get altpin: %s\n",
				    prog, argv[i], strerror(errno));
				res++;
			} else {
				if (argc > optind + 1)
					printf("%s: ", argv[i]);
				puts(altpin ? "enabled" : "disabled");
			}
			break;
		}

		if (dflag && ioctl(fd, DIGIIO_DEBUG, &debug) != 0) {
			fprintf(stderr, "%s: %s: debug: %s\n",
			    prog, argv[i], strerror(errno));
			res++;
		}

		if (iflag) {
			if (ioctl(fd, DIGIIO_MODEL, &model) != 0) {
				fprintf(stderr, "%s: %s: model: %s\n",
				    prog, argv[i], strerror(errno));
				res++;
			} else if (ioctl(fd, DIGIIO_IDENT, &name) != 0) {
				fprintf(stderr, "%s: %s: ident: %s\n",
				    prog, argv[i], strerror(errno));
				res++;
			} else {
				if (argc > optind + 1)
					printf("%s: ", argv[i]);
				printf("%s (type %d)\n", name, (int)model);
			}
		}

		if (rflag && ioctl(fd, DIGIIO_REINIT) != 0) {
			fprintf(stderr, "%s: %s: reinit: %s\n",
			    prog, argv[i], strerror(errno));
			res++;
		}

		close(fd);
	}

	return (res);
}