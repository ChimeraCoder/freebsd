
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
 * fsinfo
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <fsi_data.h>
#include <fsinfo.h>
#include <fsi_gram.h>

/* globals */
char **g_argv;
char *autodir = "/a";
char *progname;
char hostname[MAXHOSTNAMELEN + 1];
char *username;
char idvbuf[1024];
dict *dict_of_hosts;
dict *dict_of_volnames;
int errors;
int file_io_errors;
int parse_errors;
int verbose;
qelem *list_of_automounts;
qelem *list_of_hosts;

/*
 * Output file prefixes
 */
char *bootparams_pref;
char *dumpset_pref;
char *exportfs_pref;
char *fstab_pref;
char *mount_pref;


/*
 * Argument cracking...
 */
static void
fsi_get_args(int c, char *v[])
{
  int ch;
  int usage = 0;
  char *iptr = idvbuf;

  /*
   * Determine program name
   */
  if (v[0]) {
    progname = strrchr(v[0], '/');
    if (progname && progname[1])
      progname++;
    else
      progname = v[0];
  }

  if (!progname)
    progname = "fsinfo";

  while ((ch = getopt(c, v, "a:b:d:e:f:h:m:D:U:I:qv")) != -1)

    switch (ch) {

    case 'a':
      autodir = optarg;
      break;

    case 'b':
      if (bootparams_pref)
	fatal("-b option specified twice");
      bootparams_pref = optarg;
      break;

    case 'd':
      if (dumpset_pref)
	fatal("-d option specified twice");
      dumpset_pref = optarg;
      break;

    case 'h':
      xstrlcpy(hostname, optarg, sizeof(hostname));
      break;

    case 'e':
      if (exportfs_pref)
	fatal("-e option specified twice");
      exportfs_pref = optarg;
      break;

    case 'f':
      if (fstab_pref)
	fatal("-f option specified twice");
      fstab_pref = optarg;
      break;

    case 'm':
      if (mount_pref)
	fatal("-m option specified twice");
      mount_pref = optarg;
      break;

    case 'q':
      verbose = -1;
      break;

    case 'v':
      verbose = 1;
      break;

    case 'I':
    case 'D':
    case 'U':
      /* sizeof(iptr) is actually that of idvbuf.  See declaration above */
      xsnprintf(iptr, sizeof(idvbuf), "-%c%s ", ch, optarg);
      iptr += strlen(iptr);
      break;

    default:
      usage++;
      break;
    }

  if (c != optind) {
    g_argv = v + optind - 1;
#ifdef yywrap
    if (yywrap())
#endif /* yywrap */
      fatal("Cannot read any input files");
  } else {
    usage++;
  }

  if (usage) {
    fprintf(stderr,
	    "\
Usage: %s [-v] [-a autodir] [-h hostname] [-b bootparams] [-d dumpsets]\n\
\t[-e exports] [-f fstabs] [-m automounts]\n\
\t[-I dir] [-D|-U string[=string]] config ...\n", progname);
    exit(1);
  }

  if (g_argv[0])
    fsi_log("g_argv[0] = %s", g_argv[0]);
  else
    fsi_log("g_argv[0] = (nil)");
}


/*
 * Determine username of caller
 */
static char *
find_username(void)
{
  const char *u = getlogin();

  if (!u) {
    struct passwd *pw = getpwuid(getuid());
    if (pw)
      u = pw->pw_name;
  }

  if (!u)
    u = getenv("USER");
  if (!u)
    u = getenv("LOGNAME");
  if (!u)
    u = "root";

  return strdup(u);
}


/*
 * MAIN
 */
int
main(int argc, char *argv[])
{
  /*
   * Process arguments
   */
  fsi_get_args(argc, argv);

  /*
   * If no hostname given then use the local name
   */
  if (!*hostname && gethostname(hostname, sizeof(hostname)) < 0) {
    perror("gethostname");
    exit(1);
  }
  hostname[sizeof(hostname) - 1] = '\0';

  /*
   * Get the username
   */
  username = find_username();

  /*
   * New hosts and automounts
   */
  list_of_hosts = new_que();
  list_of_automounts = new_que();

  /*
   * New dictionaries
   */
  dict_of_volnames = new_dict();
  dict_of_hosts = new_dict();

  /*
   * Parse input
   */
  show_area_being_processed("read config", 11);
  if (yyparse())
    errors = 1;
  errors += file_io_errors + parse_errors;

  if (errors == 0) {
    /*
     * Do semantic analysis of input
     */
    analyze_hosts(list_of_hosts);
    analyze_automounts(list_of_automounts);
  }

  /*
   * Give up if errors
   */
  if (errors == 0) {
    /*
     * Output data files
     */

    write_atab(list_of_automounts);
    write_bootparams(list_of_hosts);
    write_dumpset(list_of_hosts);
    write_exportfs(list_of_hosts);
    write_fstab(list_of_hosts);
  }
  col_cleanup(1);

  exit(errors);
  return errors; /* should never reach here */
}