
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
 * Get info from the system namespace
 *
 * NOTE: Cannot handle reads back through the automounter.
 * THIS WILL CAUSE A DEADLOCK!
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

#define	UNION_PREFIX	"union:"
#define	UNION_PREFLEN	6

/* forward declarations */
int union_init(mnt_map *m, char *map, time_t *tp);
int union_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp);
int union_reload(mnt_map *m, char *map, void (*fn) (mnt_map *, char *, char *));


/*
 * No way to probe - check the map name begins with "union:"
 */
int
union_init(mnt_map *m, char *map, time_t *tp)
{
  *tp = 0;
  return NSTREQ(map, UNION_PREFIX, UNION_PREFLEN) ? 0 : ENOENT;
}


int
union_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp)
{
  char *mapd = strdup(map + UNION_PREFLEN);
  char **v = strsplit(mapd, ':', '\"');
  char **p;
  size_t l;

  for (p = v; p[1]; p++) ;
  l = strlen(*p) + 5;
  *pval = xmalloc(l);
  xsnprintf(*pval, l, "fs:=%s", *p);
  XFREE(mapd);
  XFREE(v);
  return 0;
}


int
union_reload(mnt_map *m, char *map, void (*fn) (mnt_map *, char *, char *))
{
  char *mapd = strdup(map + UNION_PREFLEN);
  char **v = strsplit(mapd, ':', '\"');
  char **dir;

  /*
   * Add fake /defaults entry
   */
  (*fn) (m, strdup("/defaults"), strdup("type:=link;opts:=nounmount;sublink:=${key}"));

  for (dir = v; *dir; dir++) {
    size_t l;
    struct dirent *dp;

    DIR *dirp = opendir(*dir);
    if (!dirp) {
      plog(XLOG_USER, "Cannot read directory %s: %m", *dir);
      continue;
    }
    l = strlen(*dir) + 5;

    dlog("Reading directory %s...", *dir);
    while ((dp = readdir(dirp))) {
      char *val, *dpname = &dp->d_name[0];
      if (dpname[0] == '.' &&
	  (dpname[1] == '\0' ||
	   (dpname[1] == '.' && dpname[2] == '\0')))
	continue;

      dlog("... gives %s", dp->d_name);
      val = xmalloc(l);
      xsnprintf(val, l + 5, "fs:=%s", *dir);
      (*fn) (m, strdup(dp->d_name), val);
    }
    closedir(dirp);
  }

  /*
   * Add wildcard entry
   */
  {
    size_t l = strlen(*(dir-1)) + 5;
    char *val = xmalloc(l);

    xsnprintf(val, l, "fs:=%s", *(dir-1));
    (*fn) (m, strdup("*"), val);
  }
  XFREE(mapd);
  XFREE(v);
  return 0;
}