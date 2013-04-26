
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
usage(void)
{

	fprintf(stderr, "usage: %s [-sx] [-n count] [prefix [suffix]]\n",
	    getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int c, count, linepos, maxcount, pretty, radix;

	maxcount = 0;
	pretty = 0;
	radix = 10;
	while ((c = getopt(argc, argv, "n:sx")) != -1) {
		switch (c) {
		case 'n':	/* Max. number of bytes per line. */
			maxcount = strtol(optarg, NULL, 10);
			break;
		case 's':	/* Be more style(9) comliant. */
			pretty = 1;
			break;
		case 'x':	/* Print hexadecimal numbers. */
			radix = 16;
			break;
		case '?':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		printf("%s\n", argv[0]);
	count = linepos = 0;
	while((c = getchar()) != EOF) {
		if (count) {
			putchar(',');
			linepos++;
		}
		if ((maxcount == 0 && linepos > 70) ||
		    (maxcount > 0 && count >= maxcount)) {
			putchar('\n');
			count = linepos = 0;
		}
		if (pretty) {
			if (count) {
				putchar(' ');
				linepos++;
			} else {
				putchar('\t');
				linepos += 8;
			}
		}
		switch (radix) {
		case 10:
			linepos += printf("%d", c);
			break;
		case 16:
			linepos += printf("0x%02x", c);
			break;
		default:
			abort();
		}
		count++;
	}
	putchar('\n');
	if (argc > 1)
		printf("%s\n", argv[1]);
	return (0);
}