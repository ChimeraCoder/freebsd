
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
static char sccsid[] = "@(#)tdel.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/file.h>
#include <db.h>
#include <stdio.h>

#define INITIAL	25000
#define MAXWORDS    25000	       /* # of elements in search table */

/* Usage: thash pagesize fillfactor file */
char	wp1[8192];
char	wp2[8192];
main(argc, argv)
char **argv;
{
	DBT item, key;
	DB	*dbp;
	HASHINFO ctl;
	FILE *fp;
	int	stat;

	int i = 0;

	argv++;
	ctl.nelem = INITIAL;
	ctl.hash = NULL;
	ctl.bsize = atoi(*argv++);
	ctl.ffactor = atoi(*argv++);
	ctl.cachesize = 1024 * 1024;	/* 1 MEG */
	ctl.lorder = 0;
	argc -= 2;
	if (!(dbp = dbopen( NULL, O_CREAT|O_RDWR, 0400, DB_HASH, &ctl))) {
		/* create table */
		fprintf(stderr, "cannot create: hash table size %d)\n",
			INITIAL);
		exit(1);
	}

	key.data = wp1;
	item.data = wp2;
	while ( fgets(wp1, 8192, stdin) &&
		fgets(wp2, 8192, stdin) &&
		i++ < MAXWORDS) {
/*
* put info in structure, and structure in the item
*/
		key.size = strlen(wp1);
		item.size = strlen(wp2);

/*
 * enter key/data pair into the table
 */
		if ((dbp->put)(dbp, &key, &item, R_NOOVERWRITE) != NULL) {
			fprintf(stderr, "cannot enter: key %s\n",
				item.data);
			exit(1);
		}
	}

	if ( --argc ) {
		fp = fopen ( argv[0], "r");
		i = 0;
		while ( fgets(wp1, 8192, fp) &&
			fgets(wp2, 8192, fp) &&
			i++ < MAXWORDS) {
		    key.size = strlen(wp1);
		    stat = (dbp->del)(dbp, &key, 0);
		    if (stat) {
			fprintf ( stderr, "Error retrieving %s\n", key.data );
			exit(1);
		    }
		}
		fclose(fp);
	}
	(dbp->close)(dbp);
	exit(0);
}