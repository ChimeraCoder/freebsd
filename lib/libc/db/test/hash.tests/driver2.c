
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
static char sccsid[] = "@(#)driver2.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Test driver, to try to tackle the large ugly-split problem.
 */

#include <sys/file.h>
#include <stdio.h>
#include "ndbm.h"

int my_hash(key, len)
	char	*key;
	int	len;
{
	return(17);		/* So I'm cruel... */
}

main(argc, argv)
	int	argc;
{
	DB	*db;
	DBT	key, content;
	char	keybuf[2049];
	char	contentbuf[2049];
	char	buf[256];
	int	i;
	HASHINFO	info;

	info.bsize = 1024;
	info.ffactor = 5;
	info.nelem = 1;
	info.cachesize = NULL;
#ifdef HASH_ID_PROGRAM_SPECIFIED
	info.hash_id = HASH_ID_PROGRAM_SPECIFIED;
	info.hash_func = my_hash;
#else
	info.hash = my_hash;
#endif
	info.lorder = 0;
	if (!(db = dbopen("bigtest", O_RDWR | O_CREAT, 0644, DB_HASH, &info))) {
		sprintf(buf, "dbopen: failed on file bigtest");
		perror(buf);
		exit(1);
	}
	srandom(17);
	key.data = keybuf;
	content.data = contentbuf;
	bzero(keybuf, sizeof(keybuf));
	bzero(contentbuf, sizeof(contentbuf));
	for (i=1; i <= 500; i++) {
		key.size = 128 + (random()&1023);
		content.size = 128 + (random()&1023);
/*		printf("%d: Key size %d, data size %d\n", i, key.size,
		       content.size); */
		sprintf(keybuf, "Key #%d", i);
		sprintf(contentbuf, "Contents #%d", i);
		if ((db->put)(db, &key, &content, R_NOOVERWRITE)) {
			sprintf(buf, "dbm_store #%d", i);
			perror(buf);
		}
	}
	if ((db->close)(db)) {
		perror("closing hash file");
		exit(1);
	}
	exit(0);
}