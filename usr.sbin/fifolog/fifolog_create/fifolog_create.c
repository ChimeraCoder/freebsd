
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
#include <unistd.h>
#include <err.h>
#include <libutil.h>

#include "libfifolog.h"

#define DEF_RECSIZE	512
#define DEF_RECCNT	(24 * 60 * 60)

static void
usage(void)
{
	fprintf(stderr, "Usage: fifolog_create [-l record-size] "
	    "[-r record-count] [-s size] file\n");
	exit(EX_USAGE);
}

int
main(int argc, char * const *argv)
{
	int ch;
	int64_t size;
	int64_t recsize;
	int64_t reccnt;
	const char *s;

	recsize = 0;
	size = 0;
	reccnt = 0;
	while((ch = getopt(argc, argv, "l:r:s:")) != -1) {
		switch (ch) {
		case 'l':
			if (expand_number(optarg, &recsize))
				err(1, "Couldn't parse -l argument");
			break;
		case 'r':
			if (expand_number(optarg, &reccnt))
				err(1, "Couldn't parse -r argument");
			break;
		case 's':
			if (expand_number(optarg, &size))
				err(1, "Couldn't parse -s argument");
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 1)
		usage();

	if (size != 0 && reccnt != 0 && recsize != 0) {		/* N N N */
		if (size !=  reccnt * recsize)
			errx(1, "Inconsistent -l, -r and -s values");
	} else if (size != 0 && reccnt != 0 && recsize == 0) {	/* N N Z */
		if (size % reccnt)
			errx(1,
			    "Inconsistent -r and -s values (gives remainder)");
		recsize = size / reccnt;
	} else if (size != 0 && reccnt == 0 && recsize != 0) {	/* N Z N */
		if (size % recsize)
		    errx(1, "-s arg not divisible by -l arg");
	} else if (size != 0 && reccnt == 0 && recsize == 0) {	/* N Z Z */
		recsize = DEF_RECSIZE;
		if (size % recsize)
		    errx(1, "-s arg not divisible by %jd", recsize);
	} else if (size == 0 && reccnt != 0 && recsize != 0) {	/* Z N N */
		size = reccnt * recsize;
	} else if (size == 0 && reccnt != 0 && recsize == 0) {	/* Z N Z */
		recsize = DEF_RECSIZE;
		size = reccnt * recsize;
	} else if (size == 0 && reccnt == 0 && recsize != 0) {	/* Z Z N */
		size = DEF_RECCNT * recsize;
	} else if (size == 0 && reccnt == 0 && recsize == 0) {	/* Z Z Z */
		recsize = DEF_RECSIZE;
		size = DEF_RECCNT * recsize;
	}

	s = fifolog_create(argv[0], size, recsize);
	if (s == NULL)
		return (0);
	err(1, "%s", s);
}