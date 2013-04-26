
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
"@(#) Copyright (c) 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)uuencode.c	8.2 (Berkeley) 4/2/94";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * uuencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <err.h>
#include <libgen.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void encode(void);
static void base64_encode(void);
static void usage(void);

static FILE *output;
static int mode;
static char **av;

int
main(int argc, char *argv[])
{
	struct stat sb;
	int base64;
	int ch;
	char *outfile;

	base64 = 0;
	outfile = NULL;

	if (strcmp(basename(argv[0]), "b64encode") == 0)
		base64 = 1;

	while ((ch = getopt(argc, argv, "mo:")) != -1) {
		switch (ch) {
		case 'm':
			base64 = 1;
			break;
		case 'o':
			outfile = optarg;
			break;
		case '?':
		default:
			usage();
		}
	}
	argv += optind;
	argc -= optind;

	switch(argc) {
	case 2:			/* optional first argument is input file */
		if (!freopen(*argv, "r", stdin) || fstat(fileno(stdin), &sb))
			err(1, "%s", *argv);
#define	RWX	(S_IRWXU|S_IRWXG|S_IRWXO)
		mode = sb.st_mode & RWX;
		++argv;
		break;
	case 1:
#define	RW	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
		mode = RW & ~umask(RW);
		break;
	case 0:
	default:
		usage();
	}

	av = argv;

	if (outfile != NULL) {
		output = fopen(outfile, "w+");
		if (output == NULL)
			err(1, "unable to open %s for output", outfile);
	} else
		output = stdout;
	if (base64)
		base64_encode();
	else
		encode();
	if (ferror(output))
		errx(1, "write error");
	exit(0);
}

/* ENC is the basic 1 character encoding function to make a char printing */
#define	ENC(c) ((c) ? ((c) & 077) + ' ': '`')

/*
 * Copy from in to out, encoding in base64 as you go along.
 */
static void
base64_encode(void)
{
	/*
	 * Output must fit into 80 columns, chunks come in 4, leave 1.
	 */
#define	GROUPS	((80 / 4) - 1)
	unsigned char buf[3];
	char buf2[sizeof(buf) * 2 + 1];
	size_t n;
	int rv, sequence;

	sequence = 0;

	fprintf(output, "begin-base64 %o %s\n", mode, *av);
	while ((n = fread(buf, 1, sizeof(buf), stdin))) {
		++sequence;
		rv = b64_ntop(buf, n, buf2, (sizeof(buf2) / sizeof(buf2[0])));
		if (rv == -1)
			errx(1, "b64_ntop: error encoding base64");
		fprintf(output, "%s%s", buf2, (sequence % GROUPS) ? "" : "\n");
	}
	if (sequence % GROUPS)
		fprintf(output, "\n");
	fprintf(output, "====\n");
}

/*
 * Copy from in to out, encoding as you go along.
 */
static void
encode(void)
{
	register int ch, n;
	register char *p;
	char buf[80];

	(void)fprintf(output, "begin %o %s\n", mode, *av);
	while ((n = fread(buf, 1, 45, stdin))) {
		ch = ENC(n);
		if (fputc(ch, output) == EOF)
			break;
		for (p = buf; n > 0; n -= 3, p += 3) {
			/* Pad with nulls if not a multiple of 3. */
			if (n < 3) {
				p[2] = '\0';
				if (n < 2)
					p[1] = '\0';
			}
			ch = *p >> 2;
			ch = ENC(ch);
			if (fputc(ch, output) == EOF)
				break;
			ch = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
			ch = ENC(ch);
			if (fputc(ch, output) == EOF)
				break;
			ch = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
			ch = ENC(ch);
			if (fputc(ch, output) == EOF)
				break;
			ch = p[2] & 077;
			ch = ENC(ch);
			if (fputc(ch, output) == EOF)
				break;
		}
		if (fputc('\n', output) == EOF)
			break;
	}
	if (ferror(stdin))
		errx(1, "read error");
	(void)fprintf(output, "%c\nend\n", ENC('\0'));
}

static void
usage(void)
{
	(void)fprintf(stderr,
"usage: uuencode [-m] [-o outfile] [infile] remotefile\n"
"       b64encode [-o outfile] [infile] remotefile\n");
	exit(1);
}