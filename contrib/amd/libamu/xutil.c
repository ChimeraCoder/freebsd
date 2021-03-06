
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
 * Miscellaneous Utilities: Logging, TTY, timers, signals, RPC, memory, etc.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amu.h>

/*
 * Logfp is the default logging device, and is initialized to stderr by
 * default in dplog/plog below, and in
 * amd/amfs_program.c:amfs_program_exec().
 */
FILE *logfp = NULL;

static char *am_progname = "unknown";	/* "amd" */
static char am_hostname[MAXHOSTNAMELEN] = "unknown"; /* Hostname */
pid_t am_mypid = -1;		/* process ID */
serv_state amd_state;		/* amd's state */
int foreground = 1;		/* 1 == this is the top-level server */
int debug_flags = 0;

#ifdef HAVE_SYSLOG
int syslogging;
#endif /* HAVE_SYSLOG */
int xlog_level = XLOG_ALL & ~XLOG_MAP & ~XLOG_STATS;
int xlog_level_init = ~0;
static int amd_program_number = AMQ_PROGRAM;

#ifdef DEBUG_MEM
# if defined(HAVE_MALLINFO) && defined(HAVE_MALLOC_VERIFY)
static int mem_bytes;
static int orig_mem_bytes;
# endif /* not defined(HAVE_MALLINFO) && defined(HAVE_MALLOC_VERIFY) */
#endif /* DEBUG_MEM */

/* forward definitions */
/* for GCC format string auditing */
static void real_plog(int lvl, const char *fmt, va_list vargs)
     __attribute__((__format__(__printf__, 2, 0)));


#ifdef DEBUG
/*
 * List of debug options.
 */
struct opt_tab dbg_opt[] =
{
  {"all", D_ALL},		/* All non-disruptive options */
  {"amq", D_AMQ},		/* Don't register for AMQ program */
  {"daemon", D_DAEMON},		/* Don't enter daemon mode */
  {"fork", D_FORK},		/* Don't fork server */
  {"full", D_FULL},		/* Program trace */
#ifdef HAVE_CLOCK_GETTIME
  {"hrtime", D_HRTIME},		/* Print high resolution time stamps */
#endif /* HAVE_CLOCK_GETTIME */
  /* info service specific debugging (hesiod, nis, etc) */
  {"info", D_INFO},
  {"mem", D_MEM},		/* Trace memory allocations */
  {"mtab", D_MTAB},		/* Use local mtab file */
  {"readdir", D_READDIR},	/* Check on browsable_dirs progress */
  {"str", D_STR},		/* Debug string munging */
  {"test", D_TEST},		/* Full debug - no daemon, no amq, local mtab */
  {"trace", D_TRACE},		/* Protocol trace */
  {"xdrtrace", D_XDRTRACE},	/* Trace xdr routines */
  {0, 0}
};
#endif /* DEBUG */

/*
 * List of log options
 */
struct opt_tab xlog_opt[] =
{
  {"all", XLOG_ALL},		/* All messages */
#ifdef DEBUG
  {"debug", XLOG_DEBUG},	/* Debug messages */
#endif /* DEBUG */		/* DEBUG */
  {"error", XLOG_ERROR},	/* Non-fatal system errors */
  {"fatal", XLOG_FATAL},	/* Fatal errors */
  {"info", XLOG_INFO},		/* Information */
  {"map", XLOG_MAP},		/* Map errors */
  {"stats", XLOG_STATS},	/* Additional statistical information */
  {"user", XLOG_USER},		/* Non-fatal user errors */
  {"warn", XLOG_WARNING},	/* Warnings */
  {"warning", XLOG_WARNING},	/* Warnings */
  {0, 0}
};


void
am_set_progname(char *pn)
{
  am_progname = pn;
}


const char *
am_get_progname(void)
{
  return am_progname;
}


void
am_set_hostname(char *hn)
{
  xstrlcpy(am_hostname, hn, MAXHOSTNAMELEN);
}


const char *
am_get_hostname(void)
{
  return am_hostname;
}


pid_t
am_set_mypid(void)
{
  am_mypid = getpid();
  return am_mypid;
}


long
get_server_pid()
{
  return (long) (foreground ? am_mypid : getppid());
}


voidp
xmalloc(int len)
{
  voidp p;
  int retries = 600;

  /*
   * Avoid malloc's which return NULL for malloc(0)
   */
  if (len == 0)
    len = 1;

  do {
    p = (voidp) malloc((unsigned) len);
    if (p) {
      if (amuDebug(D_MEM))
	plog(XLOG_DEBUG, "Allocated size %d; block %p", len, p);
      return p;
    }
    if (retries > 0) {
      plog(XLOG_ERROR, "Retrying memory allocation");
      sleep(1);
    }
  } while (--retries);

  plog(XLOG_FATAL, "Out of memory");
  going_down(1);

  abort();

  return 0;
}


/* like xmalloc, but zeros out the bytes */
voidp
xzalloc(int len)
{
  voidp p = xmalloc(len);

  if (p)
    memset(p, 0, len);
  return p;
}


voidp
xrealloc(voidp ptr, int len)
{
  if (amuDebug(D_MEM))
    plog(XLOG_DEBUG, "Reallocated size %d; block %p", len, ptr);

  if (len == 0)
    len = 1;

  if (ptr)
    ptr = (voidp) realloc(ptr, (unsigned) len);
  else
    ptr = (voidp) xmalloc((unsigned) len);

  if (!ptr) {
    plog(XLOG_FATAL, "Out of memory in realloc");
    going_down(1);
    abort();
  }
  return ptr;
}


#ifdef DEBUG_MEM
void
dxfree(char *file, int line, voidp ptr)
{
  if (amuDebug(D_MEM))
    plog(XLOG_DEBUG, "Free in %s:%d: block %p", file, line, ptr);
  /* this is the only place that must NOT use XFREE()!!! */
  free(ptr);
  ptr = NULL;			/* paranoid */
}


# if defined(HAVE_MALLINFO) && defined(HAVE_MALLOC_VERIFY)
static void
checkup_mem(void)
{
  struct mallinfo mi = mallinfo();
  u_long uordbytes = mi.uordblks * 4096;

  if (mem_bytes != uordbytes) {
    if (orig_mem_bytes == 0)
      mem_bytes = orig_mem_bytes = uordbytes;
    else {
      fprintf(logfp, "%s[%ld]: ", am_get_progname(), (long) am_mypid);
      if (mem_bytes < uordbytes) {
	fprintf(logfp, "ALLOC: %ld bytes", uordbytes - mem_bytes);
      } else {
	fprintf(logfp, "FREE: %ld bytes", mem_bytes - uordbytes);
      }
      mem_bytes = uordbytes;
      fprintf(logfp, ", making %d missing\n", mem_bytes - orig_mem_bytes);
    }
  }
  malloc_verify();
}
# endif /* not defined(HAVE_MALLINFO) && defined(HAVE_MALLOC_VERIFY) */
#endif /* DEBUG_MEM */


/*
 * Take a log format string and expand occurrences of %m
 * with the current error code taken from errno.  Make sure
 * 'e' never gets longer than maxlen characters.
 */
static const char *
expand_error(const char *f, char *e, size_t maxlen)
{
  const char *p;
  char *q;
  int error = errno;
  int len = 0;

  for (p = f, q = e; (*q = *p) && (size_t) len < maxlen; len++, q++, p++) {
    if (p[0] == '%' && p[1] == 'm') {
      xstrlcpy(q, strerror(error), maxlen);
      len += strlen(q) - 1;
      q += strlen(q) - 1;
      p++;
    }
  }
  e[maxlen-1] = '\0';		/* null terminate, to be sure */
  return e;
}


/*
 * Output the time of day and hostname to the logfile
 */
static void
show_time_host_and_name(int lvl)
{
  static time_t last_t = 0;
  static char *last_ctime = 0;
  time_t t;
#if defined(HAVE_CLOCK_GETTIME) && defined(DEBUG)
  struct timespec ts;
#endif /* defined(HAVE_CLOCK_GETTIME) && defined(DEBUG) */
  char nsecs[11];		/* '.' + 9 digits + '\0' */
  char *sev;

  nsecs[0] = '\0';

#if defined(HAVE_CLOCK_GETTIME) && defined(DEBUG)
  /*
   * Some systems (AIX 4.3) seem to implement clock_gettime() as stub
   * returning ENOSYS.
   */
  if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
    t = ts.tv_sec;
    if (amuDebug(D_HRTIME))
      xsnprintf(nsecs, sizeof(nsecs), ".%09ld", ts.tv_nsec);
  }
  else
#endif /* defined(HAVE_CLOCK_GETTIME) && defined(DEBUG) */
    t = clocktime(NULL);

  if (t != last_t) {
    last_ctime = ctime(&t);
    last_t = t;
  }

  switch (lvl) {
  case XLOG_FATAL:
    sev = "fatal:";
    break;
  case XLOG_ERROR:
    sev = "error:";
    break;
  case XLOG_USER:
    sev = "user: ";
    break;
  case XLOG_WARNING:
    sev = "warn: ";
    break;
  case XLOG_INFO:
    sev = "info: ";
    break;
  case XLOG_DEBUG:
    sev = "debug:";
    break;
  case XLOG_MAP:
    sev = "map:  ";
    break;
  case XLOG_STATS:
    sev = "stats:";
    break;
  default:
    sev = "hmm:  ";
    break;
  }
  fprintf(logfp, "%15.15s%s %s %s[%ld]/%s ",
	  last_ctime + 4, nsecs, am_get_hostname(),
	  am_get_progname(),
	  (long) am_mypid,
	  sev);
}


#ifdef DEBUG
/*
 * Switch on/off debug options
 */
int
debug_option(char *opt)
{
  return cmdoption(opt, dbg_opt, &debug_flags);
}


void
dplog(const char *fmt, ...)
{
  va_list ap;

  if (!logfp)
    logfp = stderr;		/* initialize before possible first use */

  va_start(ap, fmt);
  real_plog(XLOG_DEBUG, fmt, ap);
  va_end(ap);
}
#endif /* DEBUG */


void
plog(int lvl, const char *fmt, ...)
{
  va_list ap;

  if (!logfp)
    logfp = stderr;		/* initialize before possible first use */

  va_start(ap, fmt);
  real_plog(lvl, fmt, ap);
  va_end(ap);
}


static void
real_plog(int lvl, const char *fmt, va_list vargs)
{
  char msg[1024];
  char efmt[1024];
  char *ptr = msg;
  static char last_msg[1024];
  static int last_count = 0, last_lvl = 0;

  if (!(xlog_level & lvl))
    return;

#ifdef DEBUG_MEM
# if defined(HAVE_MALLINFO) && defined(HAVE_MALLOC_VERIFY)
  checkup_mem();
# endif /* not defined(HAVE_MALLINFO) && defined(HAVE_MALLOC_VERIFY) */
#endif /* DEBUG_MEM */

  /*
   * Note: xvsnprintf() may call plog() if a truncation happened, but the
   * latter has some code to break out of an infinite loop.  See comment in
   * xsnprintf() below.
   */
  xvsnprintf(ptr, 1023, expand_error(fmt, efmt, 1024), vargs);

  ptr += strlen(ptr);
  if (*(ptr-1) == '\n')
    *--ptr = '\0';

#ifdef HAVE_SYSLOG
  if (syslogging) {
    switch (lvl) {		/* from mike <mcooper@usc.edu> */
    case XLOG_FATAL:
      lvl = LOG_CRIT;
      break;
    case XLOG_ERROR:
      lvl = LOG_ERR;
      break;
    case XLOG_USER:
      lvl = LOG_WARNING;
      break;
    case XLOG_WARNING:
      lvl = LOG_WARNING;
      break;
    case XLOG_INFO:
      lvl = LOG_INFO;
      break;
    case XLOG_DEBUG:
      lvl = LOG_DEBUG;
      break;
    case XLOG_MAP:
      lvl = LOG_DEBUG;
      break;
    case XLOG_STATS:
      lvl = LOG_INFO;
      break;
    default:
      lvl = LOG_ERR;
      break;
    }
    syslog(lvl, "%s", msg);
    return;
  }
#endif /* HAVE_SYSLOG */

  *ptr++ = '\n';
  *ptr = '\0';

  /*
   * mimic syslog behavior: only write repeated strings if they differ
   */
  switch (last_count) {
  case 0:			/* never printed at all */
    last_count = 1;
    if (strlcpy(last_msg, msg, 1024) >= 1024) /* don't use xstrlcpy here (recursive!) */
      fprintf(stderr, "real_plog: string \"%s\" truncated to \"%s\"\n", last_msg, msg);
    last_lvl = lvl;
    show_time_host_and_name(lvl); /* mimic syslog header */
    fwrite(msg, ptr - msg, 1, logfp);
    fflush(logfp);
    break;

  case 1:			/* item printed once, if same, don't repeat */
    if (STREQ(last_msg, msg)) {
      last_count++;
    } else {			/* last msg printed once, new one differs */
      /* last_count remains at 1 */
      if (strlcpy(last_msg, msg, 1024) >= 1024) /* don't use xstrlcpy here (recursive!) */
	fprintf(stderr, "real_plog: string \"%s\" truncated to \"%s\"\n", last_msg, msg);
      last_lvl = lvl;
      show_time_host_and_name(lvl); /* mimic syslog header */
      fwrite(msg, ptr - msg, 1, logfp);
      fflush(logfp);
    }
    break;

  case 100:
    /*
     * Don't allow repetitions longer than 100, so you can see when something
     * cycles like crazy.
     */
    show_time_host_and_name(last_lvl);
    xsnprintf(last_msg, sizeof(last_msg),
	      "last message repeated %d times\n", last_count);
    fwrite(last_msg, strlen(last_msg), 1, logfp);
    fflush(logfp);
    last_count = 0;		/* start from scratch */
    break;

  default:			/* item repeated multiple times */
    if (STREQ(last_msg, msg)) {
      last_count++;
    } else {		/* last msg repeated+skipped, new one differs */
      show_time_host_and_name(last_lvl);
      xsnprintf(last_msg, sizeof(last_msg),
		"last message repeated %d times\n", last_count);
      fwrite(last_msg, strlen(last_msg), 1, logfp);
      if (strlcpy(last_msg, msg, 1024) >= 1024) /* don't use xstrlcpy here (recursive!) */
	fprintf(stderr, "real_plog: string \"%s\" truncated to \"%s\"\n", last_msg, msg);
      last_count = 1;
      last_lvl = lvl;
      show_time_host_and_name(lvl); /* mimic syslog header */
      fwrite(msg, ptr - msg, 1, logfp);
      fflush(logfp);
    }
    break;
  }

}


/*
 * Display current debug options
 */
void
show_opts(int ch, struct opt_tab *opts)
{
  int i;
  int s = '{';

  fprintf(stderr, "\t[-%c {no}", ch);
  for (i = 0; opts[i].opt; i++) {
    fprintf(stderr, "%c%s", s, opts[i].opt);
    s = ',';
  }
  fputs("}]\n", stderr);
}


int
cmdoption(char *s, struct opt_tab *optb, int *flags)
{
  char *p = s;
  int errs = 0;

  while (p && *p) {
    int neg;
    char *opt;
    struct opt_tab *dp, *dpn = 0;

    s = p;
    p = strchr(p, ',');
    if (p)
      *p = '\0';

    /* check for "no" prefix to options */
    if (s[0] == 'n' && s[1] == 'o') {
      opt = s + 2;
      neg = 1;
    } else {
      opt = s;
      neg = 0;
    }

    /*
     * Scan the array of debug options to find the
     * corresponding flag value.  If it is found
     * then set (or clear) the flag (depending on
     * whether the option was prefixed with "no").
     */
    for (dp = optb; dp->opt; dp++) {
      if (STREQ(opt, dp->opt))
	break;
      if (opt != s && !dpn && STREQ(s, dp->opt))
	dpn = dp;
    }

    if (dp->opt || dpn) {
      if (!dp->opt) {
	dp = dpn;
	neg = !neg;
      }
      if (neg)
	*flags &= ~dp->flag;
      else
	*flags |= dp->flag;
    } else {
      /*
       * This will log to stderr when parsing the command line
       * since any -l option will not yet have taken effect.
       */
      plog(XLOG_USER, "option \"%s\" not recognized", s);
      errs++;
    }

    /*
     * Put the comma back
     */
    if (p)
      *p++ = ',';
  }

  return errs;
}


/*
 * Switch on/off logging options
 */
int
switch_option(char *opt)
{
  int xl = xlog_level;
  int rc = cmdoption(opt, xlog_opt, &xl);

  if (rc) {
    rc = EINVAL;
  } else {
    /*
     * Keep track of initial log level, and
     * don't allow options to be turned off.
     */
    if (xlog_level_init == ~0)
      xlog_level_init = xl;
    else
      xl |= xlog_level_init;
    xlog_level = xl;
  }
  return rc;
}


#ifdef LOG_DAEMON
/*
 * get syslog facility to use.
 * logfile can be "syslog", "syslog:daemon", "syslog:local7", etc.
 */
static int
get_syslog_facility(const char *logfile)
{
  char *facstr;

  /* parse facility string */
  facstr = strchr(logfile, ':');
  if (!facstr)			/* log file was "syslog" */
    return LOG_DAEMON;
  facstr++;
  if (!facstr || facstr[0] == '\0') { /* log file was "syslog:" */
    plog(XLOG_WARNING, "null syslog facility, using LOG_DAEMON");
    return LOG_DAEMON;
  }

#ifdef LOG_KERN
  if (STREQ(facstr, "kern"))
      return LOG_KERN;
#endif /* not LOG_KERN */
#ifdef LOG_USER
  if (STREQ(facstr, "user"))
      return LOG_USER;
#endif /* not LOG_USER */
#ifdef LOG_MAIL
  if (STREQ(facstr, "mail"))
      return LOG_MAIL;
#endif /* not LOG_MAIL */

  if (STREQ(facstr, "daemon"))
      return LOG_DAEMON;

#ifdef LOG_AUTH
  if (STREQ(facstr, "auth"))
      return LOG_AUTH;
#endif /* not LOG_AUTH */
#ifdef LOG_SYSLOG
  if (STREQ(facstr, "syslog"))
      return LOG_SYSLOG;
#endif /* not LOG_SYSLOG */
#ifdef LOG_LPR
  if (STREQ(facstr, "lpr"))
      return LOG_LPR;
#endif /* not LOG_LPR */
#ifdef LOG_NEWS
  if (STREQ(facstr, "news"))
      return LOG_NEWS;
#endif /* not LOG_NEWS */
#ifdef LOG_UUCP
  if (STREQ(facstr, "uucp"))
      return LOG_UUCP;
#endif /* not LOG_UUCP */
#ifdef LOG_CRON
  if (STREQ(facstr, "cron"))
      return LOG_CRON;
#endif /* not LOG_CRON */
#ifdef LOG_LOCAL0
  if (STREQ(facstr, "local0"))
      return LOG_LOCAL0;
#endif /* not LOG_LOCAL0 */
#ifdef LOG_LOCAL1
  if (STREQ(facstr, "local1"))
      return LOG_LOCAL1;
#endif /* not LOG_LOCAL1 */
#ifdef LOG_LOCAL2
  if (STREQ(facstr, "local2"))
      return LOG_LOCAL2;
#endif /* not LOG_LOCAL2 */
#ifdef LOG_LOCAL3
  if (STREQ(facstr, "local3"))
      return LOG_LOCAL3;
#endif /* not LOG_LOCAL3 */
#ifdef LOG_LOCAL4
  if (STREQ(facstr, "local4"))
      return LOG_LOCAL4;
#endif /* not LOG_LOCAL4 */
#ifdef LOG_LOCAL5
  if (STREQ(facstr, "local5"))
      return LOG_LOCAL5;
#endif /* not LOG_LOCAL5 */
#ifdef LOG_LOCAL6
  if (STREQ(facstr, "local6"))
      return LOG_LOCAL6;
#endif /* not LOG_LOCAL6 */
#ifdef LOG_LOCAL7
  if (STREQ(facstr, "local7"))
      return LOG_LOCAL7;
#endif /* not LOG_LOCAL7 */

  /* didn't match anything else */
  plog(XLOG_WARNING, "unknown syslog facility \"%s\", using LOG_DAEMON", facstr);
  return LOG_DAEMON;
}
#endif /* not LOG_DAEMON */


/*
 * Change current logfile
 */
int
switch_to_logfile(char *logfile, int old_umask, int truncate_log)
{
  FILE *new_logfp = stderr;

  if (logfile) {
#ifdef HAVE_SYSLOG
    syslogging = 0;
#endif /* HAVE_SYSLOG */

    if (STREQ(logfile, "/dev/stderr"))
      new_logfp = stderr;
    else if (NSTREQ(logfile, "syslog", strlen("syslog"))) {

#ifdef HAVE_SYSLOG
      syslogging = 1;
      new_logfp = stderr;
      openlog(am_get_progname(),
	      LOG_PID
# ifdef LOG_NOWAIT
	      | LOG_NOWAIT
# endif /* LOG_NOWAIT */
# ifdef LOG_DAEMON
	      , get_syslog_facility(logfile)
# endif /* LOG_DAEMON */
	      );
#else /* not HAVE_SYSLOG */
      plog(XLOG_WARNING, "syslog option not supported, logging unchanged");
#endif /* not HAVE_SYSLOG */

    } else {			/* regular log file */
      (void) umask(old_umask);
      if (truncate_log)
	truncate(logfile, 0);
      new_logfp = fopen(logfile, "a");
      umask(0);
    }
  }

  /*
   * If we couldn't open a new file, then continue using the old.
   */
  if (!new_logfp && logfile) {
    plog(XLOG_USER, "%s: Can't open logfile: %m", logfile);
    return 1;
  }

  /*
   * Close the previous file
   */
  if (logfp && logfp != stderr)
    (void) fclose(logfp);
  logfp = new_logfp;

  if (logfile)
    plog(XLOG_INFO, "switched to logfile \"%s\"", logfile);
  else
    plog(XLOG_INFO, "no logfile defined; using stderr");

  return 0;
}


void
unregister_amq(void)
{
  if (!amuDebug(D_AMQ)) {
    /* find which instance of amd to unregister */
    u_long amd_prognum = get_amd_program_number();

    if (pmap_unset(amd_prognum, AMQ_VERSION) != 1)
      dlog("failed to de-register Amd program %lu, version %lu",
	   amd_prognum, AMQ_VERSION);
  }
}


void
going_down(int rc)
{
  if (foreground) {
    if (amd_state != Start) {
      if (amd_state != Done)
	return;
      unregister_amq();
    }
  }

#ifdef MOUNT_TABLE_ON_FILE
  /*
   * Call unlock_mntlist to free any important resources such as an on-disk
   * lock file (/etc/mtab~).
   */
  unlock_mntlist();
#endif /* MOUNT_TABLE_ON_FILE */

  if (foreground) {
    plog(XLOG_INFO, "Finishing with status %d", rc);
  } else {
    dlog("background process exiting with status %d", rc);
  }
  /* bye bye... */
  exit(rc);
}


/* return the rpc program number under which amd was used */
int
get_amd_program_number(void)
{
  return amd_program_number;
}


/* set the rpc program number used for amd */
void
set_amd_program_number(int program)
{
  amd_program_number = program;
}


/*
 * Release the controlling tty of the process pid.
 *
 * Algorithm: try these in order, if available, until one of them
 * succeeds: setsid(), ioctl(fd, TIOCNOTTY, 0).
 * Do not use setpgid(): on some OSs it may release the controlling tty,
 * even if the man page does not mention it, but on other OSs it does not.
 * Also avoid setpgrp(): it works on some systems, and on others it is
 * identical to setpgid().
 */
void
amu_release_controlling_tty(void)
{
  int fd;

  /*
   * In daemon mode, leaving open file descriptors to terminals or pipes
   * can be a really bad idea.
   * Case in point: the redhat startup script calls us through their 'initlog'
   * program, which exits as soon as the original amd process exits. If,
   * at some point, a misbehaved library function decides to print something
   * to the screen, we get a SIGPIPE and die.
   * And guess what: NIS glibc functions will attempt to print to stderr
   * "YPBINDPROC_DOMAIN: Domain not bound" if ypbind is running but can't find
   * a ypserver.
   *
   * So we close all of our "terminal" filedescriptors, i.e. 0, 1 and 2, then
   * reopen them as /dev/null.
   *
   * XXX We should also probably set the SIGPIPE handler to SIG_IGN.
   */
  fd = open("/dev/null", O_RDWR);
  if (fd < 0) {
    plog(XLOG_WARNING, "Could not open /dev/null for rw: %m");
  } else {
    fflush(stdin);  close(0); dup2(fd, 0);
    fflush(stdout); close(1); dup2(fd, 1);
    fflush(stderr); close(2); dup2(fd, 2);
    close(fd);
  }

#ifdef HAVE_SETSID
  /* XXX: one day maybe use vhangup(2) */
  if (setsid() < 0) {
    plog(XLOG_WARNING, "Could not release controlling tty using setsid(): %m");
  } else {
    plog(XLOG_INFO, "released controlling tty using setsid()");
    return;
  }
#endif /* HAVE_SETSID */

#ifdef TIOCNOTTY
  fd = open("/dev/tty", O_RDWR);
  if (fd < 0) {
    /* not an error if already no controlling tty */
    if (errno != ENXIO)
      plog(XLOG_WARNING, "Could not open controlling tty: %m");
  } else {
    if (ioctl(fd, TIOCNOTTY, 0) < 0 && errno != ENOTTY)
      plog(XLOG_WARNING, "Could not disassociate tty (TIOCNOTTY): %m");
    else
      plog(XLOG_INFO, "released controlling tty using ioctl(TIOCNOTTY)");
    close(fd);
  }
  return;
#endif /* not TIOCNOTTY */

  plog(XLOG_ERROR, "unable to release controlling tty");
}


/* setup a single signal handler */
void
setup_sighandler(int signum, void (*handler)(int))
{
#ifdef HAVE_SIGACTION
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_flags = 0;		/* unnecessary */
  sa.sa_handler = handler;
  sigemptyset(&(sa.sa_mask));	/* probably unnecessary too */
  sigaddset(&(sa.sa_mask), signum);
  sigaction(signum, &sa, NULL);
#else /* not HAVE_SIGACTION */
  (void) signal(signum, handler);
#endif /* not HAVE_SIGACTION */
}


/*
 * Return current time in seconds.  If passed a non-null argyument, then
 * fill it in with the current time in seconds and microseconds (useful
 * for mtime updates).
 */
time_t
clocktime(nfstime *nt)
{
  static struct timeval now;	/* keep last time, as default */

  if (gettimeofday(&now, NULL) < 0) {
    plog(XLOG_ERROR, "clocktime: gettimeofday: %m");
    /* hack: force time to have incremented by at least 1 second */
    now.tv_sec++;
  }
  /* copy seconds and microseconds. may demote a long to an int */
  if (nt) {
    nt->nt_seconds = (u_int) now.tv_sec;
    nt->nt_useconds = (u_int) now.tv_usec;
  }
  return (time_t) now.tv_sec;
}


/*
 * Make all the directories in the path.
 */
int
mkdirs(char *path, int mode)
{
  /*
   * take a copy in case path is in readonly store
   */
  char *p2 = strdup(path);
  char *sp = p2;
  struct stat stb;
  int error_so_far = 0;

  /*
   * Skip through the string make the directories.
   * Mostly ignore errors - the result is tested at the end.
   *
   * This assumes we are root so that we can do mkdir in a
   * mode 555 directory...
   */
  while ((sp = strchr(sp + 1, '/'))) {
    *sp = '\0';
    if (mkdir(p2, mode) < 0) {
      error_so_far = errno;
    } else {
      dlog("mkdir(%s)", p2);
    }
    *sp = '/';
  }

  if (mkdir(p2, mode) < 0) {
    error_so_far = errno;
  } else {
    dlog("mkdir(%s)", p2);
  }

  XFREE(p2);

  return stat(path, &stb) == 0 &&
    (stb.st_mode & S_IFMT) == S_IFDIR ? 0 : error_so_far;
}


/*
 * Remove as many directories in the path as possible.
 * Give up if the directory doesn't appear to have
 * been created by Amd (not mode dr-x) or an rmdir
 * fails for any reason.
 */
void
rmdirs(char *dir)
{
  char *xdp = strdup(dir);
  char *dp;

  do {
    struct stat stb;
    /*
     * Try to find out whether this was
     * created by amd.  Do this by checking
     * for owner write permission.
     */
    if (stat(xdp, &stb) == 0 && (stb.st_mode & 0200) == 0) {
      if (rmdir(xdp) < 0) {
	if (errno != ENOTEMPTY &&
	    errno != EBUSY &&
	    errno != EEXIST &&
	    errno != EROFS &&
	    errno != EINVAL)
	  plog(XLOG_ERROR, "rmdir(%s): %m", xdp);
	break;
      } else {
	dlog("rmdir(%s)", xdp);
      }
    } else {
      break;
    }

    dp = strrchr(xdp, '/');
    if (dp)
      *dp = '\0';
  } while (dp && dp > xdp);

  XFREE(xdp);
}