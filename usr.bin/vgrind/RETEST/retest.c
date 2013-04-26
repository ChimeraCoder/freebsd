
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
static char copyright[] =
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)retest.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include <ctype.h>

int l_onecase = 0;
char * _start;
char * _escaped;
char * convexp();
char * expmatch();
main()
{
    char reg[132];
    char *ireg;
    char str[132];
    char *match;
    char matstr[132];
    char c;

    while (1) {
	printf ("\nexpr: ");
	scanf ("%s", reg);
	ireg = convexp(reg);
	match = ireg;
	while(*match) {
	    switch (*match) {

	    case '\\':
	    case '(':
	    case ')':
	    case '|':
		printf ("%c", *match);
		break;

	    default:
		if (isalnum(*match))
		    printf("%c", *match);
		else
		    printf ("<%03o>", *match);
		break;
	    }
	    match++;
	}
	printf("\n");
	getchar();
	while(1) {
	    printf ("string: ");
	    match = str;
	    while ((c = getchar()) != '\n')
		*match++ = c;
	    *match = 0;
	    if (str[0] == '#')
		break;
	    matstr[0] = 0;
	    _start = str;
	    _escaped = 0;
	    match = expmatch (str, ireg, matstr);
	    if (match == 0)
		printf ("FAILED\n");
	    else
		printf ("match\nmatstr = %s\n", matstr);
	}

    }
}