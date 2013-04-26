
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
"@(#) Copyright (c) 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if 0
#ifndef lint
static char sccsid[] = "@(#)lptest.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif

#include "lp.cdefs.h"		/* A cross-platform version of <sys/cdefs.h> */
__FBSDID("$FreeBSD$");

#include <stdlib.h>
#include <stdio.h>
#include <err.h>

/*
 * lptest -- line printer test program (and other devices).
 */
int
main(int argc, char **argv)
{
	int len, count;
	register int i, j, fc, nc;
	char outbuf[BUFSIZ];

	setbuf(stdout, outbuf);
	if (argc >= 2)
		len = atoi(argv[1]);
	else
		len = 79;
	if (argc >= 3)
		count = atoi(argv[2]);
	else
		count = 200;
	fc = ' ';
	for (i = 0; i < count; i++) {
		if (++fc == 0177)
			fc = ' ';
		nc = fc;
		for (j = 0; j < len; j++) {
			if (putchar(nc) == EOF)
				err(1, "Write error");
			if (++nc == 0177)
				nc = ' ';
		}
		if (putchar('\n') == EOF)
			err(1, "Write error");
	}
	(void) fflush(stdout);
	exit(0);
}