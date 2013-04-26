
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ndbm.c	8.4 (Berkeley) 7/21/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * This package provides a dbm compatible interface to the new hashing
 * package described in db(3).
 */

#include <sys/param.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <ndbm.h>
#include "hash.h"

/*
 * Returns:
 * 	*DBM on success
 *	 NULL on failure
 */
extern DBM *
dbm_open(file, flags, mode)
	const char *file;
	int flags, mode;
{
	HASHINFO info;
	char path[MAXPATHLEN];

	info.bsize = 4096;
	info.ffactor = 40;
	info.nelem = 1;
	info.cachesize = 0;
	info.hash = NULL;
	info.lorder = 0;

	if( strlen(file) >= sizeof(path) - strlen(DBM_SUFFIX)) {
		errno = ENAMETOOLONG;
		return(NULL);
	}
	(void)strcpy(path, file);
	(void)strcat(path, DBM_SUFFIX);
	return ((DBM *)__hash_open(path, flags, mode, &info, 0));
}

extern void
dbm_close(db)
	DBM *db;
{
	(void)(db->close)(db);
}

/*
 * Returns:
 *	DATUM on success
 *	NULL on failure
 */
extern datum
dbm_fetch(db, key)
	DBM *db;
	datum key;
{
	datum retdata;
	int status;
	DBT dbtkey, dbtretdata;

	dbtkey.data = key.dptr;
	dbtkey.size = key.dsize;
	status = (db->get)(db, &dbtkey, &dbtretdata, 0);
	if (status) {
		dbtretdata.data = NULL;
		dbtretdata.size = 0;
	}
	retdata.dptr = dbtretdata.data;
	retdata.dsize = dbtretdata.size;
	return (retdata);
}

/*
 * Returns:
 *	DATUM on success
 *	NULL on failure
 */
extern datum
dbm_firstkey(db)
	DBM *db;
{
	int status;
	datum retkey;
	DBT dbtretkey, dbtretdata;

	status = (db->seq)(db, &dbtretkey, &dbtretdata, R_FIRST);
	if (status)
		dbtretkey.data = NULL;
	retkey.dptr = dbtretkey.data;
	retkey.dsize = dbtretkey.size;
	return (retkey);
}

/*
 * Returns:
 *	DATUM on success
 *	NULL on failure
 */
extern datum
dbm_nextkey(db)
	DBM *db;
{
	int status;
	datum retkey;
	DBT dbtretkey, dbtretdata;

	status = (db->seq)(db, &dbtretkey, &dbtretdata, R_NEXT);
	if (status)
		dbtretkey.data = NULL;
	retkey.dptr = dbtretkey.data;
	retkey.dsize = dbtretkey.size;
	return (retkey);
}

/*
 * Returns:
 *	 0 on success
 *	<0 failure
 */
extern int
dbm_delete(db, key)
	DBM *db;
	datum key;
{
	int status;
	DBT dbtkey;

	dbtkey.data = key.dptr;
	dbtkey.size = key.dsize;
	status = (db->del)(db, &dbtkey, 0);
	if (status)
		return (-1);
	else
		return (0);
}

/*
 * Returns:
 *	 0 on success
 *	<0 failure
 *	 1 if DBM_INSERT and entry exists
 */
extern int
dbm_store(db, key, data, flags)
	DBM *db;
	datum key, data;
	int flags;
{
	DBT dbtkey, dbtdata;

	dbtkey.data = key.dptr;
	dbtkey.size = key.dsize;
	dbtdata.data = data.dptr;
	dbtdata.size = data.dsize;
	return ((db->put)(db, &dbtkey, &dbtdata,
	    (flags == DBM_INSERT) ? R_NOOVERWRITE : 0));
}

extern int
dbm_error(db)
	DBM *db;
{
	HTAB *hp;

	hp = (HTAB *)db->internal;
	return (hp->error);
}

extern int
dbm_clearerr(db)
	DBM *db;
{
	HTAB *hp;

	hp = (HTAB *)db->internal;
	hp->error = 0;
	return (0);
}

extern int
dbm_dirfno(db)
	DBM *db;
{
	return(((HTAB *)db->internal)->fp);
}