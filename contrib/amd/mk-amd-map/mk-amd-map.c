
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
 * Convert a file map into an ndbm map
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>

/* (libdb version 2) uses .db extensions but an old dbm API */
/* check for libgdbm to distinguish it from linux systems */
#if defined(DBM_SUFFIX) && !defined(HAVE_LIBGDBM)
# define HAVE_DB_SUFFIX
#endif /* not defined(DBM_SUFFIX) && !defined(HAVE_LIBGDBM) */

#ifdef HAVE_MAP_NDBM

static int
store_data(voidp db, char *k, char *v)
{
  datum key, val;

  key.dptr = k;
  val.dptr = v;
  key.dsize = strlen(k) + 1;
  val.dsize = strlen(v) + 1;
  return dbm_store((DBM *) db, key, val, DBM_INSERT);
}


/*
 * Read one line from file.
 */
static int
read_line(char *buf, int size, FILE *fp)
{
  int done = 0;

  do {
    while (fgets(buf, size, fp)) {
      int len = strlen(buf);

      done += len;
      if (len > 1 && buf[len - 2] == '\\' && buf[len - 1] == '\n') {
	int ch;
	buf += len - 2;
	size -= len - 2;
	*buf = '\n';
	buf[1] = '\0';

	/*
	 * Skip leading white space on next line
	 */
	while ((ch = getc(fp)) != EOF && isascii(ch) && isspace(ch)) ;
	(void) ungetc(ch, fp);
      } else {
	return done;
      }
    }
  } while (size > 0 && !feof(fp));

  return done;
}


/*
 * Read through a map.
 */
static int
read_file(FILE *fp, char *map, voidp db)
{
  char key_val[2048];
  int chuck = 0;
  int line_no = 0;
  int errs = 0;

  while (read_line(key_val, 2048, fp)) {
    char *kp;
    char *cp;
    char *hash;
    int len = strlen(key_val);

    line_no++;

    /*
     * Make sure we got the whole line
     */
    if (key_val[len - 1] != '\n') {
      fprintf(stderr, "line %d in \"%s\" is too long", line_no, map);
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
     * Check whether key matches, or whether
     * the entry is a wildcard entry.
     */
    if (*cp)
      *cp++ = '\0';
    while (*cp && isascii(*cp) && isspace((int)*cp))
      cp++;
    if (*kp == '+') {
      fprintf(stderr, "Can't interpolate %s\n", kp);
      errs++;
    } else if (*cp) {
      if (db) {
	if (store_data(db, kp, cp) < 0) {
	  fprintf(stderr, "Could store %s -> %s\n", kp, cp);
	  errs++;
	}
      } else {
	printf("%s\t%s\n", kp, cp);
      }
    } else {
      fprintf(stderr, "%s: line %d has no value field", map, line_no);
      errs++;
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
  return errs;
}


static int
remove_file(char *f)
{
  if (unlink(f) < 0 && errno != ENOENT)
    return -1;

  return 0;
}


int
main(int argc, char *argv[])
{
  FILE *mapf;			/* the input file to read from */
  int error;
  char *mapsrc;
  DBM *db = NULL;
  static char maptmp[] = "dbmXXXXXX";
#ifdef HAVE_DB_SUFFIX
  char maptdb[16];
  char *map_name_db = (char *) NULL;
#else /* not HAVE_DB_SUFFIX */
  char maptpag[16], maptdir[16];
  char *map_name_pag = (char *) NULL, *map_name_dir = (char *) NULL;
#endif /* not HAVE_DB_SUFFIX */
  size_t l = 0;
  char *sl;
  int printit = 0;
  int usage = 0;
  int ch;
  extern int optind;

  /* test options */
  while ((ch = getopt(argc, argv, "p")) != -1)
    switch (ch) {
    case 'p':
      printit = 1;
      break;
    default:
      usage++;
      break;
    }

  if (usage || optind != (argc - 1)) {
    fputs("Usage: mk-amd-map [-p] file-map\n", stderr);
    exit(1);
  }
  mapsrc = argv[optind];

  /* test if can get to the map directory */
  sl = strrchr(mapsrc, '/');
  if (sl) {
    *sl = '\0';
    if (chdir(mapsrc) < 0) {
      fputs("Can't chdir to ", stderr);
      perror(mapsrc);
      exit(1);
    }
    mapsrc = sl + 1;
  }

  /* open source file */
  mapf = fopen(mapsrc, "r");
  if (!mapf) {
    fprintf(stderr, "cannot open source file ");
    perror(mapsrc);
    exit(1);
  }

#ifndef DEBUG
  signal(SIGINT, SIG_IGN);
#endif /* DEBUG */

  if (!printit) {
    /* enough space for ".db" or ".pag" or ".dir" appended */
    l = strlen(mapsrc) + 5;
#ifdef HAVE_DB_SUFFIX
    map_name_db = (char *) malloc(l);
    error = (map_name_db == NULL);
#else /* not HAVE_DB_SUFFIX */
    map_name_pag = (char *) malloc(l);
    map_name_dir = (char *) malloc(l);
    error = (map_name_pag == NULL || map_name_dir == NULL);
#endif /* not HAVE_DB_SUFFIX */
    if (error) {
      perror("mk-amd-map: malloc");
      exit(1);
    }

#ifdef HAVE_MKSTEMP
    {
      /*
       * XXX: hack to avoid compiler complaints about mktemp not being
       * secure, since we have to do a dbm_open on this anyway.  So use
       * mkstemp if you can, and then close the fd, but we get a safe
       * and unique file name.
       */
      int dummyfd;
      dummyfd = mkstemp(maptmp);
      if (dummyfd >= 0)
	close(dummyfd);
    }
#else /* not HAVE_MKSTEMP */
    mktemp(maptmp);
#endif /* not HAVE_MKSTEMP */

    /* remove existing temps (if any) */
#ifdef HAVE_DB_SUFFIX
    xsnprintf(maptdb, sizeof(maptdb), "%s.db", maptmp);
    if (remove_file(maptdb) < 0) {
      fprintf(stderr, "Can't remove existing temporary file; ");
      perror(maptdb);
      exit(1);
    }
#else /* not HAVE_DB_SUFFIX */
    xsnprintf(maptpag, sizeof(maptpag), "%s.pag", maptmp);
    xsnprintf(maptdir, sizeof(maptdir), "%s.dir", maptmp);
    if (remove_file(maptpag) < 0 || remove_file(maptdir) < 0) {
      fprintf(stderr, "Can't remove existing temporary files; %s and ", maptpag);
      perror(maptdir);
      exit(1);
    }
#endif /* not HAVE_DB_SUFFIX */

    db = dbm_open(maptmp, O_RDWR|O_CREAT|O_EXCL, 0444);
    if (!db) {
      fprintf(stderr, "cannot initialize temporary database: %s", maptmp);
      exit(1);
    }
  }

  /* print db to stdout or to temp database */
  error = read_file(mapf, mapsrc, db);
  fclose(mapf);
  if (error) {
    if (printit)
      fprintf(stderr, "Error reading source file  %s\n", mapsrc);
    else
      fprintf(stderr, "Error creating database map for %s\n", mapsrc);
    exit(1);
  }

  if (printit)
    exit(0);			/* nothing more to do */

  /* if gets here, we wrote to a database */

  dbm_close(db);
  /* all went well */

#ifdef HAVE_DB_SUFFIX
  /* sizeof(map_name_db) is malloc'ed above */
  xsnprintf(map_name_db, l, "%s.db", mapsrc);
  if (rename(maptdb, map_name_db) < 0) {
    fprintf(stderr, "Couldn't rename %s to ", maptdb);
    perror(map_name_db);
    /* Throw away the temporary map */
    unlink(maptdb);
    exit(1);
  }
#else /* not HAVE_DB_SUFFIX */
  /* sizeof(map_name_{pag,dir}) are malloc'ed above */
  xsnprintf(map_name_pag, l, "%s.pag", mapsrc);
  xsnprintf(map_name_dir, l, "%s.dir", mapsrc);
  if (rename(maptpag, map_name_pag) < 0) {
    fprintf(stderr, "Couldn't rename %s to ", maptpag);
    perror(map_name_pag);
    /* Throw away the temporary map */
    unlink(maptpag);
    unlink(maptdir);
    exit(1);
  }
  if (rename(maptdir, map_name_dir) < 0) {
    fprintf(stderr, "Couldn't rename %s to ", maptdir);
    perror(map_name_dir);
    /* remove the (presumably bad) .pag file */
    unlink(map_name_pag);
    /* throw away remaining part of original map */
    unlink(map_name_dir);
    /* throw away the temporary map */
    unlink(maptdir);
    fprintf(stderr, "WARNING: existing map \"%s.{dir,pag}\" destroyed\n",
	    mapsrc);
    exit(1);
  }
#endif /* not HAVE_DB_SUFFIX */

  exit(0);
}

#else /* not HAVE_MAP_NDBM */

int
main()
{
  fputs("mk-amd-map: This system does not support hashed database files\n", stderr);
  exit(1);
}

#endif /* not HAVE_MAP_NDBM */