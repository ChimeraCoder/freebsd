
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

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)hexdump.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hexdump.h"

FS *fshead;				/* head of format strings */
int blocksize;				/* data block size */
int exitval;				/* final exit value */
int length = -1;			/* max bytes to read */

int
main(int argc, char *argv[])
{
	FS *tfs;
	char *p;

	(void)setlocale(LC_ALL, "");

	if (!(p = strrchr(argv[0], 'o')) || strcmp(p, "od"))
		newsyntax(argc, &argv);
	else
		oldsyntax(argc, &argv);

	/* figure out the data block size */
	for (blocksize = 0, tfs = fshead; tfs; tfs = tfs->nextfs) {
		tfs->bcnt = size(tfs);
		if (blocksize < tfs->bcnt)
			blocksize = tfs->bcnt;
	}
	/* rewrite the rules, do syntax checking */
	for (tfs = fshead; tfs; tfs = tfs->nextfs)
		rewrite(tfs);

	(void)next(argv);
	display();
	exit(exitval);
}