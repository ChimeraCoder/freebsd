
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
 * Get info from executable map
 *
 * Original from Erik Kline, 2004.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

#define	MAX_LINE_LEN		1500

/* forward declarations */
int exec_init(mnt_map *m, char *map, time_t *tp);
int exec_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp);


/*
 * a timed fgets()
 */
static char *
fgets_timed(char *s, int size, int rdfd, int secs)
{
  fd_set fds;
  struct timeval timeo;
  time_t start, now;
  int rval=0, i=0;

  if (!s || size < 0 || rdfd < 0)
    return 0;

  s[0] = 0;
  if (size == 0)
    return s;

  start = clocktime(NULL);
  while (s[i] != '\n'  &&  i < size-1) {
    s[i+1] = 0; /* places the requisite trailing '\0' */

    /* ready for reading */
    rval = read(rdfd, (void *)(s+i), 1);
    if (rval == 1) {
      if (s[i] == 0) {
        rval = 0;
        break;
      }
      i++;
      continue;
    } else if (rval == 0) {
      break;
    } else if (rval < 0  &&  errno != EAGAIN  &&  errno != EINTR) {
      plog(XLOG_WARNING, "fgets_timed read error: %m");
      break;
    }

    timeo.tv_usec = 0;
    now = clocktime(NULL) - start;
    if (secs <= 0)
      timeo.tv_sec = 0;
    else if (now < secs)
      timeo.tv_sec = secs - now;
    else {
      /* timed out (now>=secs) */
      plog(XLOG_WARNING, "executable map read timed out (> %d secs)", secs);
      rval = -1;
      break;
    }

    FD_ZERO(&fds);
    FD_SET(rdfd, &fds);

    rval = select(rdfd+1, &fds, 0, 0, &timeo);
    if (rval < 0) {
      /* error selecting */
      plog(XLOG_WARNING, "fgets_timed select error: %m");
      if (errno == EINTR)
        continue;
      rval = -1;
      break;
    } else if (rval == 0) {
      /* timed out */
      plog(XLOG_WARNING, "executable map read timed out (> %d secs)", secs);
      rval = -1;
      break;
    }
  }

  if (rval > 0)
    return s;

  close(rdfd);
  return (rval == 0 ? s : 0);
}


static int
read_line(char *buf, int size, int fd)
{
  int done = 0;

  while (fgets_timed(buf, size, fd, gopt.exec_map_timeout)) {
    int len = strlen(buf);
    done += len;
    if (len > 1  &&  buf[len - 2] == '\\' &&
        buf[len - 1] == '\n') {
      buf += len - 2;
      size -= len - 2;
      *buf = '\n';
      buf[1] = '\0';
    } else {
      return done;
    }
  }

  return done;
}


/*
 * Try to locate a value in a query answer
 */
static int
exec_parse_qanswer(int fd, char *map, char *key, char **pval, time_t *tp)
{
  char qanswer[MAX_LINE_LEN], *dc = 0;
  int chuck = 0;
  int line_no = 0;

  while (read_line(qanswer, sizeof(qanswer), fd)) {
    char *cp;
    char *hash;
    int len = strlen(qanswer);
    line_no++;

    /*
     * Make sure we got the whole line
     */
    if (qanswer[len - 1] != '\n') {
      plog(XLOG_WARNING, "line %d in \"%s\" is too long", line_no, map);
      chuck = 1;
    } else {
      qanswer[len - 1] = '\0';
    }

    /*
     * Strip comments
     */
    hash = strchr(qanswer, '#');
    if (hash)
      *hash = '\0';

    /*
     * Find beginning of value (query answer)
     */
    for (cp = qanswer; *cp && !isascii((int)*cp) && !isspace((int)*cp); cp++)
      ;;

    /* Ignore blank lines */
    if (!*cp)
      goto again;

    /*
     * Return a copy of the data
     */
    dc = strdup(cp);
    *pval = dc;
    dlog("%s returns %s", key, dc);

    close(fd);
    return 0;

  again:
    /*
     * If the last read didn't get a whole line then
     * throw away the remainder before continuing...
     */
    if (chuck) {
      while (fgets_timed(qanswer, sizeof(qanswer), fd, gopt.exec_map_timeout) &&
	     !strchr(qanswer, '\n')) ;
      chuck = 0;
    }
  }

  return ENOENT;
}


static int
set_nonblock(int fd)
{
  int val;

  if (fd < 0)
     return 0;

  if ((val = fcntl(fd, F_GETFL, 0)) < 0) {
    plog(XLOG_WARNING, "set_nonblock fcntl F_GETFL error: %m");
    return 0;
  }

  val |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, val) < 0) {
    plog(XLOG_WARNING, "set_nonblock fcntl F_SETFL error: %m");
    return 0;
  }

  return 1;
}


static int
exec_map_open(char *emap, char *key)
{
  pid_t p1, p2;
  int pdes[2], nullfd, i;
  char *argv[3];

  if (!emap)
    return 0;

  argv[0] = emap;
  argv[1] = key;
  argv[2] = NULL;

  if ((nullfd = open("/dev/null", O_WRONLY|O_NOCTTY)) < 0)
    return -1;

  if (pipe(pdes) < 0) {
    close(nullfd);
    return -1;
  }

  switch ((p1 = vfork())) {
  case -1:
    /* parent: fork error */
    close(nullfd);
    close(pdes[0]);
    close(pdes[1]);
    return -1;
  case 0:
    /* child #1 */
    p2 = vfork();
    switch (p2) {
    case -1:
      /* child #1: fork error */
      exit(errno);
    case 0:
      /* child #2: init will reap our status */
      if (pdes[1] != STDOUT_FILENO) {
	dup2(pdes[1], STDOUT_FILENO);
	close(pdes[1]);
      }

      if (nullfd != STDERR_FILENO) {
	dup2(nullfd, STDERR_FILENO);
	close(nullfd);
      }

      for (i=0; i<FD_SETSIZE; i++)
	if (i != STDOUT_FILENO  &&  i != STDERR_FILENO)
	  close(i);

      /* make the write descriptor non-blocking */
      if (!set_nonblock(STDOUT_FILENO)) {
	close(STDOUT_FILENO);
	exit(-1);
      }

      execve(emap, argv, NULL);
      exit(errno);		/* in case execve failed */
    }

    /* child #1 */
    exit(0);
  }

  /* parent */
  close(nullfd);
  close(pdes[1]);

  /* anti-zombie insurance */
  while (waitpid(p1,0,0) < 0)
    if (errno != EINTR)
      exit(errno);

  /* make the read descriptor non-blocking */
  if (!set_nonblock(pdes[0])) {
    close(pdes[0]);
    return -1;
  }

  return pdes[0];
}


/*
 * Check for various permissions on executable map without trying to
 * fork a new executable-map process.
 *
 * return: >0 (errno) if failed
 *          0 if ok
 */
static int
exec_check_perm(char *map)
{
  struct stat sb;

  /* sanity and permission checks */
  if (!map) {
    dlog("exec_check_permission got a NULL map");
    return EINVAL;
  }
  if (stat(map, &sb)) {
    plog(XLOG_ERROR, "map \"%s\" stat failure: %m", map);
    return errno;
  }
  if (!S_ISREG(sb.st_mode)) {
    plog(XLOG_ERROR, "map \"%s\" should be regular file", map);
    return EINVAL;
  }
  if (sb.st_uid != 0) {
    plog(XLOG_ERROR, "map \"%s\" owned by uid %u (must be 0)", map, (u_int) sb.st_uid);
    return EACCES;
  }
  if (!(sb.st_mode & S_IXUSR)) {
    plog(XLOG_ERROR, "map \"%s\" should be executable", map);
    return EACCES;
  }
  if (sb.st_mode & (S_ISUID|S_ISGID)) {
    plog(XLOG_ERROR, "map \"%s\" should not be setuid/setgid", map);
    return EACCES;
  }
  if (sb.st_mode & S_IWOTH) {
    plog(XLOG_ERROR, "map \"%s\" should not be world writeable", map);
    return EACCES;
  }

  return 0;			/* all is well */
}


int
exec_init(mnt_map *m, char *map, time_t *tp)
{
  /*
   * Basically just test that the executable map can be found
   * and has proper permissions.
   */
  return exec_check_perm(map);
}


int
exec_search(mnt_map *m, char *map, char *key, char **pval, time_t *tp)
{
  int mapfd, ret;

  if ((ret = exec_check_perm(map)) != 0) {
    return ret;
  }

  if (!key)
    return 0;

  if (logfp)
    fflush(logfp);
  dlog("exec_search \"%s\", key: \"%s\"", map, key);
  mapfd = exec_map_open(map, key);

  if (mapfd >= 0) {
    if (tp)
      *tp = clocktime(NULL);

    return exec_parse_qanswer(mapfd, map, key, pval, tp);
  }

  return errno;
}