
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

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1988, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if 0
#ifndef lint
static char sccsid[] = "@(#)env.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "envopts.h"

extern char **environ;

int	 env_verbosity;

static void usage(void);

int
main(int argc, char **argv)
{
	char *altpath, **ep, *p, **parg;
	char *cleanenv[1];
	int ch, want_clear;
	int rtrn;

	altpath = NULL;
	want_clear = 0;
	while ((ch = getopt(argc, argv, "-iP:S:u:v")) != -1)
		switch(ch) {
		case '-':
		case 'i':
			want_clear = 1;
			break;
		case 'P':
			altpath = strdup(optarg);
			break;
		case 'S':
			/*
			 * The -S option, for "split string on spaces, with
			 * support for some simple substitutions"...
			 */
			split_spaces(optarg, &optind, &argc, &argv);
			break;
		case 'u':
			if (env_verbosity)
				fprintf(stderr, "#env unset:\t%s\n", optarg);
			rtrn = unsetenv(optarg);
			if (rtrn == -1)
				err(EXIT_FAILURE, "unsetenv %s", optarg);
			break;
		case 'v':
			env_verbosity++;
			if (env_verbosity > 1)
				fprintf(stderr, "#env verbosity now at %d\n",
				    env_verbosity);
			break;
		case '?':
		default:
			usage();
		}
	if (want_clear) {
		environ = cleanenv;
		cleanenv[0] = NULL;
		if (env_verbosity)
			fprintf(stderr, "#env clearing environ\n");
	}
	for (argv += optind; *argv && (p = strchr(*argv, '=')); ++argv) {
		if (env_verbosity)
			fprintf(stderr, "#env setenv:\t%s\n", *argv);
		*p = '\0';
		rtrn = setenv(*argv, p + 1, 1);
		*p = '=';
		if (rtrn == -1)
			err(EXIT_FAILURE, "setenv %s", *argv);
	}
	if (*argv) {
		if (altpath)
			search_paths(altpath, argv);
		if (env_verbosity) {
			fprintf(stderr, "#env executing:\t%s\n", *argv);
			for (parg = argv, argc = 0; *parg; parg++, argc++)
				fprintf(stderr, "#env    arg[%d]=\t'%s'\n",
				    argc, *parg);
			if (env_verbosity > 1)
				sleep(1);
		}
		execvp(*argv, argv);
		err(errno == ENOENT ? 127 : 126, "%s", *argv);
	}
	for (ep = environ; *ep; ep++)
		(void)printf("%s\n", *ep);
	exit(0);
}

static void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: env [-iv] [-P utilpath] [-S string] [-u name]\n"
	    "           [name=value ...] [utility [argument ...]]\n");
	exit(1);
}