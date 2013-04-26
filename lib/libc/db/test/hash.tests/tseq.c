
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
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)tseq.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <db.h>

#define INITIAL	25000
#define MAXWORDS    25000	       /* # of elements in search table */


char	wp[8192];
char	cp[8192];
main(argc, argv)
char **argv;
{
	DBT item, key, res;
	DB	*dbp;
	FILE *fp;
	int	stat;

	if (!(dbp = dbopen( "hashtest", O_RDONLY, 0400, DB_HASH, NULL))) {
		/* create table */
		fprintf(stderr, "cannot open: hash table\n" );
		exit(1);
	}

/*
* put info in structure, and structure in the item
*/
	for ( stat = (dbp->seq) (dbp, &res, &item, 1 );
	      stat == 0;
	      stat = (dbp->seq) (dbp, &res, &item, 0 ) ) {

	      bcopy ( res.data, wp, res.size );
	      wp[res.size] = 0;
	      bcopy ( item.data, cp, item.size );
	      cp[item.size] = 0;

	      printf ( "%s %s\n", wp, cp );
	}
	(dbp->close)(dbp);
	exit(0);
}