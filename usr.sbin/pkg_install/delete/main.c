
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <err.h>

#include "lib.h"
#include "delete.h"

char	*Prefix		= NULL;
Boolean	CleanDirs	= FALSE;
Boolean	Interactive	= FALSE;
Boolean	NoDeInstall	= FALSE;
Boolean	Recursive	= FALSE;
match_t	MatchType	= MATCH_GLOB;

static void usage(void);

static char opts[] = "adDfGhinp:rvxX";
static struct option longopts[] = {
	{ "all",	no_argument,		NULL,		'a' },
	{ "clean-dirs",	no_argument,		NULL,		'd' },
	{ "dry-run",	no_argument,		NULL,		'n' },
	{ "extended",	no_argument,		NULL,		'X' },
	{ "force",	no_argument,		NULL,		'f' },
	{ "help",	no_argument,		NULL,		'h' },
	{ "interactive",no_argument,		NULL,		'i' },
	{ "prefix",	required_argument,	NULL,		'p' },
	{ "recursive",	no_argument,		NULL,		'r' },
	{ "regex",	no_argument,		NULL,		'x' },
	{ "no-glob",	no_argument,		NULL,		'G' },
	{ "no-script",	no_argument,		NULL,		'D' },
	{ "no-scripts",	no_argument,		NULL,		'D' },
	{ "verbose",	no_argument,		NULL,		'v' },
	{ NULL,		0,			NULL,		0 },
};

int
main(int argc, char **argv)
{
    int ch, error;
    char **pkgs, **start;
    char *pkgs_split;
    const char *tmp;
    struct stat stat_s;

    warnpkgng();
    pkgs = start = argv;
    while ((ch = getopt_long(argc, argv, opts, longopts, NULL)) != -1)
	switch(ch) {
	case 'v':
	    Verbose++;
	    break;

	case 'f':
	    Force = TRUE;
	    break;

	case 'p':
	    Prefix = optarg;
	    break;

	case 'D':
	    NoDeInstall = TRUE;
	    break;

	case 'd':
	    CleanDirs = TRUE;
	    break;

	case 'n':
	    Fake = TRUE;
	    Verbose = TRUE;
	    break;

	case 'a':
	    MatchType = MATCH_ALL;
	    break;

	case 'G':
	    MatchType = MATCH_EXACT;
	    break;

	case 'x':
	    MatchType = MATCH_REGEX;
	    break;

	case 'X':
	    MatchType = MATCH_EREGEX;
	    break;

	case 'i':
	    Interactive = TRUE;
	    break;

	case 'r':
	    Recursive = TRUE;
	    break;

	case 'h':
	default:
	    usage();
	    break;
	}

    argc -= optind;
    argv += optind;

    /* Get all the remaining package names, if any */
    while (*argv) {
	/* Don't try to apply heuristics if arguments are regexs */
	if (MatchType != MATCH_REGEX)
	    while ((pkgs_split = strrchr(*argv, (int)'/')) != NULL) {
		*pkgs_split++ = '\0';
		/*
		 * If character after the '/' is alphanumeric, then we've found the
		 * package name.  Otherwise we've come across a trailing '/' and
		 * need to continue our quest.
		 */
		if (isalnum(*pkgs_split) || ((MatchType == MATCH_GLOB) && \
		    strpbrk(pkgs_split, "*?[]") != NULL)) {
		    *argv = pkgs_split;
		    break;
		}
	    }
	*pkgs++ = *argv++;
    }

    /* If no packages, yelp */
    if (pkgs == start && MatchType != MATCH_ALL)
	warnx("missing package name(s)"), usage();
    *pkgs = NULL;
    tmp = LOG_DIR;
    (void) stat(tmp, &stat_s);
    if (!Fake && getuid() && geteuid() != stat_s.st_uid) {
	if (!Force)
	    errx(1, "you do not own %s, use -f to force", tmp);
	else
	    warnx("you do not own %s (proceeding anyways)", tmp);
    }
    if ((error = pkg_perform(start)) != 0) {
	if (Verbose)
	    warnx("%d package deletion(s) failed", error);
	return error;
    }
    else
	return 0;
}

static void
usage(void)
{
    fprintf(stderr, "%s\n%s\n",
	"usage: pkg_delete [-dDfGinrvxX] [-p prefix] pkg-name ...",
	"       pkg_delete -a [flags]");
    exit(1);
}