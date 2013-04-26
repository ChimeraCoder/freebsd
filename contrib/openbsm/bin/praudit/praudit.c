
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
 * Tool used to parse audit records conforming to the BSM structure.
 */

/*
 * praudit [-lnpx] [-r | -s] [-d del] [file ...]
 */

#include <bsm/libbsm.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char	*optarg;
extern int	 optind, optopt, opterr,optreset;

static char	*del = ",";	/* Default delimiter. */
static int	 oneline = 0;
static int	 partial = 0;
static int	 oflags = AU_OFLAG_NONE;

static void
usage(void)
{

	fprintf(stderr, "usage: praudit [-lnpx] [-r | -s] [-d del] "
	    "[file ...]\n");
	exit(1);
}

/*
 * Token printing for each token type .
 */
static int
print_tokens(FILE *fp)
{
	u_char *buf;
	tokenstr_t tok;
	int reclen;
	int bytesread;

	/* Allow tail -f | praudit to work. */
	if (partial) {
		u_char type = 0;
		/* Record must begin with a header token. */
		do {
			type = fgetc(fp);
		} while(type != AUT_HEADER32);
		ungetc(type, fp);
	}

	while ((reclen = au_read_rec(fp, &buf)) != -1) {
		bytesread = 0;
		while (bytesread < reclen) {
			/* Is this an incomplete record? */
			if (-1 == au_fetch_tok(&tok, buf + bytesread,
			    reclen - bytesread))
				break;
			au_print_flags_tok(stdout, &tok, del, oflags);
			bytesread += tok.len;
			if (oneline) {
				if (!(oflags & AU_OFLAG_XML))
					printf("%s", del);
			} else
				printf("\n");
		}
		free(buf);
		if (oneline)
			printf("\n");
		fflush(stdout);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	int ch;
	int i;
	FILE *fp;

	while ((ch = getopt(argc, argv, "d:lnprsx")) != -1) {
		switch(ch) {
		case 'd':
			del = optarg;
			break;

		case 'l':
			oneline = 1;
			break;

		case 'n':
			oflags |= AU_OFLAG_NORESOLVE;
			break;

		case 'p':
			partial = 1;
			break;

		case 'r':
			if (oflags & AU_OFLAG_SHORT)
				usage();	/* Exclusive from shortfrm. */
			oflags |= AU_OFLAG_RAW;
			break;

		case 's':
			if (oflags & AU_OFLAG_RAW)
				usage();	/* Exclusive from raw. */
			oflags |= AU_OFLAG_SHORT;
			break;

		case 'x':
			oflags |= AU_OFLAG_XML;
			break;

		case '?':
		default:
			usage();
		}
	}

	if (oflags & AU_OFLAG_XML)
		au_print_xml_header(stdout);

	/* For each of the files passed as arguments dump the contents. */
	if (optind == argc) {
		print_tokens(stdin);
		return (1);
	}
	for (i = optind; i < argc; i++) {
		fp = fopen(argv[i], "r");
		if ((fp == NULL) || (print_tokens(fp) == -1))
			perror(argv[i]);
		if (fp != NULL)
			fclose(fp);
	}

	if (oflags & AU_OFLAG_XML)
		au_print_xml_footer(stdout);

	return (1);
}