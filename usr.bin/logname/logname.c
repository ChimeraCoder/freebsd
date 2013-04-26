
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
"@(#) Copyright (c) 1991, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static const char sccsid[] = "@(#)logname.c	8.2 (Berkeley) 4/3/94";
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void usage(void);

int
main(int argc, char *argv[] __unused)
{
	char *p;

	if (argc != 1)
		usage();
	if ((p = getlogin()) == NULL)
		err(1, NULL);
	(void)printf("%s\n", p);
	exit(0);
}

void
usage(void)
{
	(void)fprintf(stderr, "usage: logname\n");
	exit(1);
}