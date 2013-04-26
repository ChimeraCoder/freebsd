
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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uuid.h>

static void
usage(void)
{
	(void)fprintf(stderr, "usage: uuidgen [-1] [-n count] [-o filename]\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	uuid_t *store, *uuid;
	char *p;
	int ch, count, i, iterate;

	count = -1;	/* no count yet */
	fp = stdout;	/* default output file */
	iterate = 0;	/* not one at a time */
	while ((ch = getopt(argc, argv, "1n:o:")) != -1)
		switch (ch) {
		case '1':
			iterate = 1;
			break;
		case 'n':
			if (count > 0)
				usage();
			count = strtol(optarg, &p, 10);
			if (*p != 0 || count < 1)
				usage();
			break;
		case 'o':
			if (fp != stdout)
				errx(1, "multiple output files not allowed");
			fp = fopen(optarg, "w");
			if (fp == NULL)
				err(1, "fopen");
			break;
		default:
			usage();
		}
	argv += optind;
	argc -= optind;

	if (argc)
		usage();

	if (count == -1)
		count = 1;

	store = (uuid_t*)malloc(sizeof(uuid_t) * count);
	if (store == NULL)
		err(1, "malloc()");

	if (!iterate) {
		/* Get them all in a single batch */
		if (uuidgen(store, count) != 0)
			err(1, "uuidgen()");
	} else {
		uuid = store;
		for (i = 0; i < count; i++) {
			if (uuidgen(uuid++, 1) != 0)
				err(1, "uuidgen()");
		}
	}

	uuid = store;
	while (count--) {
		uuid_to_string(uuid++, &p, NULL);
		fprintf(fp, "%s\n", p);
		free(p);
	}

	free(store);
	if (fp != stdout)
		fclose(fp);
	return (0);
}