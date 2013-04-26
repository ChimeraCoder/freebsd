
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
static char sccsid[] = "@(#)db.c	8.4 (Berkeley) 2/21/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>

#include <db.h>

static int __dberr(void);

DB *
dbopen(const char *fname, int flags, int mode, DBTYPE type, const void *openinfo)
{

#define	DB_FLAGS	(DB_LOCK | DB_SHMEM | DB_TXN)
#define	USE_OPEN_FLAGS							\
	(O_CREAT | O_EXCL | O_EXLOCK | O_NOFOLLOW | O_NONBLOCK | 	\
	 O_RDONLY | O_RDWR | O_SHLOCK | O_SYNC | O_TRUNC)

	if ((flags & ~(USE_OPEN_FLAGS | DB_FLAGS)) == 0)
		switch (type) {
		case DB_BTREE:
			return (__bt_open(fname, flags & USE_OPEN_FLAGS,
			    mode, openinfo, flags & DB_FLAGS));
		case DB_HASH:
			return (__hash_open(fname, flags & USE_OPEN_FLAGS,
			    mode, openinfo, flags & DB_FLAGS));
		case DB_RECNO:
			return (__rec_open(fname, flags & USE_OPEN_FLAGS,
			    mode, openinfo, flags & DB_FLAGS));
		}
	errno = EINVAL;
	return (NULL);
}

static int
__dberr(void)
{
	return (RET_ERROR);
}

/*
 * __DBPANIC -- Stop.
 *
 * Parameters:
 *	dbp:	pointer to the DB structure.
 */
void
__dbpanic(DB *dbp)
{
	/* The only thing that can succeed is a close. */
	dbp->del = (int (*)(const struct __db *, const DBT*, u_int))__dberr;
	dbp->fd = (int (*)(const struct __db *))__dberr;
	dbp->get = (int (*)(const struct __db *, const DBT*, DBT *, u_int))__dberr;
	dbp->put = (int (*)(const struct __db *, DBT *, const DBT *, u_int))__dberr;
	dbp->seq = (int (*)(const struct __db *, DBT *, DBT *, u_int))__dberr;
	dbp->sync = (int (*)(const struct __db *, u_int))__dberr;
}