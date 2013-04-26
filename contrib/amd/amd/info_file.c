
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
 * Get info from file
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

#define	MAX_LINE_LEN	1500

/* forward declarations */
int file_init_or_mtime(mnt_map *m, char *map, time_t *tp);
int file_reload(mnt_map *m, char *map, void (*fn) (mnt_map *, char *, char *));
int file_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp);


static int
read_line(char *buf, int size, FILE *fp)
{
  int done = 0;

  do {
    while (fgets(buf, size, fp)) {
      int len = strlen(buf);
      done += len;
      if (len > 1 && buf[len - 2] == '\\' &&
	  buf[len - 1] == '\n') {
	int ch;
	buf += len - 2;
	size -= len - 2;
	*buf = '\n';
	buf[1] = '\0';
	/*
	 * Skip leading white space on next line
	 */
	while ((ch = getc(fp)) != EOF &&
	       isascii(ch) && isspace(ch)) ;
	(void) ungetc(ch, fp);
      } else {
	return done;
      }
    }
  } while (size > 0 && !feof(fp) && !ferror(fp));

  return done;
}


/*
 * Try to locate a key in a file
 */
static int
file_search_or_reload(FILE *fp,
		      char *map,
		      char *key,
		      char **val,
		      mnt_map *m,
		      void (*fn) (mnt_map *m, char *, char *))
{
  char key_val[MAX_LINE_LEN];
  int chuck = 0;
  int line_no = 0;

  while (read_line(key_val, sizeof(key_val), fp)) {
    char *kp;
    char *cp;
    char *hash;
    int len = strlen(key_val);
    line_no++;

    /*
     * Make sure we got the whole line
     */
    if (key_val[len - 1] != '\n') {
      plog(XLOG_WARNING, "line %d in \"%s\" is too long", line_no, map);
      chuck = 1;
    } else {
      key_val[len - 1] = '\0';
    }

    /*
     * Strip comments
     */
    hash = strchr(key_val, '#');
    if (hash)
      *hash = '\0';

    /*
     * Find start of key
     */
    for (kp = key_val; *kp && isascii(*kp) && isspace((int)*kp); kp++) ;

    /*
     * Ignore blank lines
     */
    if (!*kp)
      goto again;

    /*
     * Find end of key
     */
    for (cp = kp; *cp && (!isascii(*cp) || !isspace((int)*cp)); cp++) ;

    /*
     * Check whether key matches
     */
    if (*cp)
      *cp++ = '\0';

    if (fn || (*key == *kp && STREQ(key, kp))) {
      while (*cp && isascii(*cp) && isspace((int)*cp))
	cp++;
      if (*cp) {
	/*
	 * Return a copy of the data
	 */
	char *dc = strdup(cp);
	if (fn) {
	  (*fn) (m, strdup(kp), dc);
	} else {
	  *val = dc;
	  dlog("%s returns %s", key, dc);
	}
	if (!fn)
	  return 0;
      } else {
	plog(XLOG_USER, "%s: line %d has no value field", map, line_no);
      }
    }

  again:
    /*
     * If the last read didn't get a whole line then
     * throw away the remainder before continuing...
     */
    if (chuck) {
      while (fgets(key_val, sizeof(key_val), fp) &&
	     !strchr(key_val, '\n')) ;
      chuck = 0;
    }
  }

  return fn ? 0 : ENOENT;
}


static FILE *
file_open(char *map, time_t *tp)
{
  FILE *mapf = fopen(map, "r");

  if (mapf && tp) {
    struct stat stb;
    if (fstat(fileno(mapf), &stb) < 0)
      *tp = clocktime(NULL);
    else
      *tp = stb.st_mtime;
  }
  return mapf;
}


int
file_init_or_mtime(mnt_map *m, char *map, time_t *tp)
{
  FILE *mapf = file_open(map, tp);

  if (mapf) {
    fclose(mapf);
    return 0;
  }
  return errno;
}


int
file_reload(mnt_map *m, char *map, void (*fn) (mnt_map *, char *, char *))
{
  FILE *mapf = file_open(map, (time_t *) 0);

  if (mapf) {
    int error = file_search_or_reload(mapf, map, 0, 0, m, fn);
    (void) fclose(mapf);
    return error;
  }
  return errno;
}


int
file_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp)
{
  time_t t;
  FILE *mapf = file_open(map, &t);

  if (mapf) {
    int error;
    if (*tp < t) {
      *tp = t;
      error = -1;
    } else {
      error = file_search_or_reload(mapf, map, key, pval, 0, 0);
    }
    (void) fclose(mapf);
    return error;
  }
  return errno;
}