
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

#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <err.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <poll.h>
#include <string.h>
#include <zlib.h>

#include "libfifolog.h"

static void
usage(void)
{
	fprintf(stderr,
	    "Usage: fifolog_writer [-w write-rate] [-s sync-rate] "
	    "[-z compression] file\n");
	exit(EX_USAGE);
}

int
main(int argc, char * const *argv)
{
	struct fifolog_writer *f;
	const char *es;
	struct pollfd pfd[1];
	char buf[BUFSIZ], *p;
	int i, c;
	unsigned w_opt = 10;
	unsigned s_opt = 60;
	unsigned z_opt = Z_BEST_COMPRESSION;

	while ((c = getopt(argc, argv, "w:s:z:")) != -1) {
		switch(c) {
		case 'w':
			w_opt = strtoul(optarg, NULL, 0);
			break;
		case 's':
			s_opt = strtoul(optarg, NULL, 0);
			break;
		case 'z':
			z_opt = strtoul(optarg, NULL, 0);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 1)
		usage();

	if (z_opt > 9)
		usage();

	if (w_opt > s_opt)
		usage();

	f = fifolog_write_new();
	assert(f != NULL);

	es = fifolog_write_open(f, argv[0], w_opt, s_opt, z_opt);
	if (es)
		err(1, "Error: %s", es);

	while (1) {
		pfd[0].fd = 0;
		pfd[0].events = POLLIN;
		i = poll(pfd, 1, 1000);
		if (i == 1) {
			if (fgets(buf, sizeof buf, stdin) == NULL)
				break;
			p = strchr(buf, '\0');
			assert(p != NULL);
			while (p > buf && isspace(p[-1]))
				p--;
			*p = '\0';
			if (*buf != '\0')
				fifolog_write_record_poll(f, 0, 0, buf, 0);
		} else if (i == 0)
			fifolog_write_poll(f, 0);
	}
	fifolog_write_close(f);
	return (0);
}