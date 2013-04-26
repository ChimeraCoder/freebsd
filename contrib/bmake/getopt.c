
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if !defined(HAVE_GETOPT) || defined(WANT_GETOPT_LONG) || defined(BROKEN_GETOPT)

#if defined(LIBC_SCCS) && !defined(lint)
/* static char sccsid[] = "from: @(#)getopt.c	8.2 (Berkeley) 4/2/94"; */
static char *rcsid = "$Id: getopt.c,v 1.3 1999/01/08 02:14:18 sjg Exp $";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

int	opterr = 1,		/* if error message should be printed */
	optind = 1,		/* index into parent argv vector */
	optopt = BADCH,		/* character checked for validity */
	optreset;		/* reset getopt */
char	*optarg;		/* argument associated with option */

/*
 * getopt --
 *	Parse argc/argv argument vector.
 */
int
getopt(nargc, nargv, ostr)
	int nargc;
	char * const *nargv;
	const char *ostr;
{
	extern char *__progname;
	static char *place = EMSG;		/* option letter processing */
	char *oli;				/* option letter list index */

#ifndef BSD4_4
	if (!__progname) {
		if (__progname = strrchr(nargv[0], '/'))
			++__progname;
		else
			__progname = nargv[0];
	}
#endif
	
	if (optreset || !*place) {		/* update scanning pointer */
		optreset = 0;
		if (optind >= nargc || *(place = nargv[optind]) != '-') {
			place = EMSG;
			return (-1);
		}
		if (place[1] && *++place == '-'	/* found "--" */
		    && !place[1]) {		/* and not "--foo" */
			++optind;
			place = EMSG;
			return (-1);
		}
	}					/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
	    !(oli = strchr(ostr, optopt))) {
		/*
		 * if the user didn't specify '-' as an option,
		 * assume it means -1.
		 */
		if (optopt == (int)'-')
			return (-1);
		if (!*place)
			++optind;
		if (opterr && *ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", __progname, optopt);
		return (BADCH);
	}
	if (*++oli != ':') {			/* don't need argument */
		optarg = NULL;
		if (!*place)
			++optind;
	}
	else {					/* need an argument */
		if (*place)			/* no white space */
			optarg = place;
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (opterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    __progname, optopt);
			return (BADCH);
		}
	 	else				/* white space */
			optarg = nargv[optind];
		place = EMSG;
		++optind;
	}
	return (optopt);			/* dump back option letter */
}
#endif
#ifdef MAIN
#ifndef BSD4_4
char *__progname;
#endif

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int c;
	char *opts = argv[1];

	--argc;
	++argv;
	
	while ((c = getopt(argc, argv, opts)) != EOF) {
		switch (c) {
		case '-':
			if (optarg)
				printf("--%s ", optarg);
			break;
		case '?':
			exit(1);
			break;
		default:
			if (optarg)
				printf("-%c %s ", c, optarg);
			else
				printf("-%c ", c);
			break;
		}
	}

	if (optind < argc) {
		printf("-- ");
		for (; optind < argc; ++optind) {
			printf("%s ", argv[optind]);
		}
	}
	printf("\n");
	exit(0);
}
#endif