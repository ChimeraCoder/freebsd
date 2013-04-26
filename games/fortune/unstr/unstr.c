
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
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static const char sccsid[] = "@(#)unstr.c     8.1 (Berkeley) 5/31/93";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 *	This program un-does what "strfile" makes, thereby obtaining the
 * original file again.  This can be invoked with the name of the output
 * file, the input file, or both. If invoked with only a single argument
 * ending in ".dat", it is pressumed to be the input file and the output
 * file will be the same stripped of the ".dat".  If the single argument
 * doesn't end in ".dat", then it is presumed to be the output file, and
 * the input file is that name prepended by a ".dat".  If both are given
 * they are treated literally as the input and output files.
 *
 *	Ken Arnold		Aug 13, 1978
 */

#include <sys/param.h>
#include <sys/endian.h>
#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strfile.h"

static char	*Infile,		/* name of input file */
		Datafile[MAXPATHLEN],	/* name of data file */
		Delimch;		/* delimiter character */

static FILE	*Inf, *Dataf;

static void order_unstr(STRFILE *);

/* ARGSUSED */
int
main(int argc, char *argv[])
{
	static STRFILE tbl;		/* description table */

	if (argc != 2) {
		fprintf(stderr, "usage: unstr datafile\n");
		exit(1);
	}
	Infile = argv[1];
	strcpy(Datafile, Infile);
	strcat(Datafile, ".dat");
	if ((Inf = fopen(Infile, "r")) == NULL)
		err(1, "%s", Infile);
	if ((Dataf = fopen(Datafile, "r")) == NULL)
		err(1, "%s", Datafile);
	fread((char *)&tbl, sizeof(tbl), 1, Dataf);
	tbl.str_version = be32toh(tbl.str_version);
	tbl.str_numstr = be32toh(tbl.str_numstr);
	tbl.str_longlen = be32toh(tbl.str_longlen);
	tbl.str_shortlen = be32toh(tbl.str_shortlen);
	tbl.str_flags = be32toh(tbl.str_flags);
	if (!(tbl.str_flags & (STR_ORDERED | STR_RANDOM)))
		errx(1, "nothing to do -- table in file order");
	Delimch = tbl.str_delim;
	order_unstr(&tbl);
	fclose(Inf);
	fclose(Dataf);
	exit(0);
}

static void
order_unstr(STRFILE *tbl)
{
	uint32_t i;
	char *sp;
	off_t pos;
	char buf[BUFSIZ];

	for (i = 0; i < tbl->str_numstr; i++) {
		fread(&pos, 1, sizeof(pos), Dataf);
		fseeko(Inf, be64toh(pos), SEEK_SET);
		if (i != 0)
			printf("%c\n", Delimch);
		for (;;) {
			sp = fgets(buf, sizeof(buf), Inf);
			if (sp == NULL || STR_ENDSTRING(sp, *tbl))
				break;
			else
				fputs(sp, stdout);
		}
	}
}