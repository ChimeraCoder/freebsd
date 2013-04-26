
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
"@(#) Copyright (c) 1987, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)yes.c	8.1 (Berkeley) 6/6/93";
#else
static const char rcsid[] = "$FreeBSD$";
#endif
#endif /* not lint */

#include <err.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	if (argc > 1)
		while (puts(argv[1]) != EOF)
			;
	else
		while (puts("y") != EOF)
			;
	err(1, "stdout");
	/*NOTREACHED*/
}