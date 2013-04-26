
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

/****************************************************************************
 *  Author: Thomas E. Dickey                        2006                    *
 ****************************************************************************/

#include <curses.priv.h>
#include <tic.h>
#include <hashed_db.h>

#if USE_HASHED_DB

MODULE_ID("$Id: hashed_db.c,v 1.13 2006/08/19 19:48:38 tom Exp $")

#if HASHED_DB_API >= 2
static DBC *cursor;
#endif

/*
 * Open the database.
 */
NCURSES_EXPORT(DB *)
_nc_db_open(const char *path, bool modify)
{
    DB *result = 0;

#if HASHED_DB_API >= 4
    db_create(&result, NULL, 0);
    result->open(result,
		 NULL,
		 path,
		 NULL,
		 DB_HASH,
		 modify ? DB_CREATE : DB_RDONLY,
		 0644);
#elif HASHED_DB_API >= 3
    db_create(&result, NULL, 0);
    result->open(result,
		 path,
		 NULL,
		 DB_HASH,
		 modify ? DB_CREATE : DB_RDONLY,
		 0644);
#elif HASHED_DB_API >= 2
    int code;

    if ((code = db_open(path,
			DB_HASH,
			modify ? DB_CREATE : DB_RDONLY,
			0644,
			(DB_ENV *) 0,
			(DB_INFO *) 0,
			&result)) != 0) {
	T(("cannot open %s: %s", path, strerror(code)));
	result = 0;
    } else {
	T(("opened %s", path));
    }
#else
    result = dbopen(path,
		    modify ? (O_CREAT | O_RDWR) : O_RDONLY,
		    0644,
		    DB_HASH,
		    NULL);
    if (result != 0) {
	T(("opened %s", path));
    }
#endif
    return result;
}

/*
 * Close the database.  Do not attempt to use the 'db' handle after this call.
 */
NCURSES_EXPORT(int)
_nc_db_close(DB * db)
{
    int result;

#if HASHED_DB_API >= 2
    result = db->close(db, 0);
#else
    result = db->close(db);
#endif
    return result;
}

/*
 * Write a record to the database.
 *
 * Returns 0 on success.
 *
 * FIXME:  the FreeBSD cap_mkdb program assumes the database could have
 * duplicates.  There appears to be no good reason for that (review/fix).
 */
NCURSES_EXPORT(int)
_nc_db_put(DB * db, DBT * key, DBT * data)
{
    int result;
#if HASHED_DB_API >= 2
    /* remove any pre-existing value, since we do not want duplicates */
    (void) db->del(db, NULL, key, 0);
    result = db->put(db, NULL, key, data, DB_NOOVERWRITE);
#else
    result = db->put(db, key, data, R_NOOVERWRITE);
#endif
    return result;
}

/*
 * Read a record from the database.
 *
 * Returns 0 on success.
 */
NCURSES_EXPORT(int)
_nc_db_get(DB * db, DBT * key, DBT * data)
{
    int result;

    memset(data, 0, sizeof(*data));
#if HASHED_DB_API >= 2
    result = db->get(db, NULL, key, data, 0);
#else
    result = db->get(db, key, data, 0);
#endif
    return result;
}

/*
 * Read the first record from the database, ignoring order.
 *
 * Returns 0 on success.
 */
NCURSES_EXPORT(int)
_nc_db_first(DB * db, DBT * key, DBT * data)
{
    int result;

    memset(key, 0, sizeof(*key));
    memset(data, 0, sizeof(*data));
#if HASHED_DB_API >= 2
    if ((result = db->cursor(db, NULL, &cursor, 0)) == 0) {
	result = cursor->c_get(cursor, key, data, DB_FIRST);
    }
#else
    result = db->seq(db, key, data, 0);
#endif
    return result;
}

/*
 * Read the next record from the database, ignoring order.
 *
 * Returns 0 on success.
 */
NCURSES_EXPORT(int)
_nc_db_next(DB * db, DBT * key, DBT * data)
{
    int result;

#if HASHED_DB_API >= 2
    (void) db;
    if (cursor != 0) {
	result = cursor->c_get(cursor, key, data, DB_NEXT);
    } else {
	result = -1;
    }
#else
    result = db->seq(db, key, data, 0);
#endif
    return result;
}

/*
 * Check if a record is a terminfo index record.  Index records are those that
 * contain only an alias pointing to a list of aliases.
 */
NCURSES_EXPORT(bool)
_nc_db_have_index(DBT * key, DBT * data, char **buffer, int *size)
{
    bool result = FALSE;
    int used = data->size - 1;
    char *have = (char *) data->data;

    (void) key;
    if (*have++ == 2) {
	result = TRUE;
    }
    /*
     * Update params in any case for consistency with _nc_db_have_data().
     */
    *buffer = have;
    *size = used;
    return result;
}

/*
 * Check if a record is the terminfo data record.  Ignore index records, e.g.,
 * those that contain only an alias pointing to a list of aliases.
 */
NCURSES_EXPORT(bool)
_nc_db_have_data(DBT * key, DBT * data, char **buffer, int *size)
{
    bool result = FALSE;
    int used = data->size - 1;
    char *have = (char *) data->data;

    if (*have++ == 0) {
	if (data->size > key->size
	    && IS_TIC_MAGIC(have)) {
	    result = TRUE;
	}
    }
    /*
     * Update params in any case to make it simple to follow a index record
     * to the data record.
     */
    *buffer = have;
    *size = used;
    return result;
}

#else

extern
NCURSES_EXPORT(void)
_nc_hashed_db(void);

NCURSES_EXPORT(void)
_nc_hashed_db(void)
{
}

#endif /* USE_HASHED_DB */