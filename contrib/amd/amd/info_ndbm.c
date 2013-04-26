
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

/*
 * Get info from NDBM map
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/* forward declarations */
int ndbm_init(mnt_map *m, char *map, time_t *tp);
int ndbm_mtime(mnt_map *m, char *map, time_t *tp);
int ndbm_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp);


static int
search_ndbm(DBM *db, char *key, char **val)
{
  datum k, v;

  k.dptr = key;
  k.dsize = strlen(key) + 1;
  v = dbm_fetch(db, k);
  if (v.dptr) {
    *val = strdup(v.dptr);
    return 0;
  }
  return ENOENT;
}


int
ndbm_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp)
{
  DBM *db;

  db = dbm_open(map, O_RDONLY, 0);
  if (db) {
    struct stat stb;
    int error;
#ifdef DBM_SUFFIX
    char dbfilename[256];

    xstrlcpy(dbfilename, map, sizeof(dbfilename));
    xstrlcat(dbfilename, DBM_SUFFIX, sizeof(dbfilename));
    error = stat(dbfilename, &stb);
#else /* not DBM_SUFFIX */
    error = fstat(dbm_pagfno(db), &stb);
#endif /* not DBM_SUFFIX */
    if (!error && *tp < stb.st_mtime) {
      *tp = stb.st_mtime;
      error = -1;
    } else {
      error = search_ndbm(db, key, pval);
    }
    (void) dbm_close(db);
    return error;
  }
  return errno;
}


int
ndbm_init(mnt_map *m, char *map, time_t *tp)
{
  DBM *db;

  db = dbm_open(map, O_RDONLY, 0);
  if (db) {
    struct stat stb;
    int error;
#ifdef DBM_SUFFIX
    char dbfilename[256];

    xstrlcpy(dbfilename, map, sizeof(dbfilename));
    xstrlcat(dbfilename, DBM_SUFFIX, sizeof(dbfilename));
    error = stat(dbfilename, &stb);
#else /* not DBM_SUFFIX */
    error = fstat(dbm_pagfno(db), &stb);
#endif /* not DBM_SUFFIX */
    if (error < 0)
      *tp = clocktime(NULL);
    else
      *tp = stb.st_mtime;
    dbm_close(db);
    return 0;
  }
  return errno;
}


int
ndbm_mtime(mnt_map *m, char *map, time_t *tp)
{
  return ndbm_init(m,map, tp);
}