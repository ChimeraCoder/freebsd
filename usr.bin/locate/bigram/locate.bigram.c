
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
static char copyright[] =
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)locate.bigram.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif

/*
 *  bigram < sorted_file_names | sort -nr | 
 *  	awk 'NR <= 128 { printf $2 }' > bigrams
 *
 * List bigrams for 'updatedb' script.
 * Use 'code' to encode a file using this output.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>			/* for MAXPATHLEN */
#include "locate.h"

u_char buf1[MAXPATHLEN] = " ";
u_char buf2[MAXPATHLEN];
u_int bigram[UCHAR_MAX + 1][UCHAR_MAX + 1];

int
main(void)
{
	u_char *cp;
	u_char *oldpath = buf1, *path = buf2;
	u_int i, j;

     	while (fgets(path, sizeof(buf2), stdin) != NULL) {

		/* 
		 * We don't need remove newline character '\n'.
		 * '\n' is less than ASCII_MIN and will be later
		 * ignored at output.
		 */


		/* skip longest common prefix */
		for (cp = path; *cp == *oldpath; cp++, oldpath++)
			if (*cp == '\0')
				break;

		while (*cp != '\0' && *(cp + 1) != '\0') {
			bigram[(u_char)*cp][(u_char)*(cp + 1)]++;
			cp += 2;
		}

		/* swap pointers */
		if (path == buf1) { 
			path = buf2;
			oldpath = buf1;
		} else {
			path = buf1;
			oldpath = buf2;
		}
   	}

	/* output, boundary check */
	for (i = ASCII_MIN; i <= ASCII_MAX; i++)
		for (j = ASCII_MIN; j <= ASCII_MAX; j++)
			if (bigram[i][j] != 0)
				(void)printf("%4u %c%c\n", bigram[i][j], i, j);

	exit(0);
}