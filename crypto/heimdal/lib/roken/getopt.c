
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getopt.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#ifndef __STDC__
#define const
#endif
#include <config.h>
#include "roken.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * get option letter from argument vector
 */
ROKEN_LIB_VARIABLE int opterr = 1; /* if error message should be printed */
ROKEN_LIB_VARIABLE int optind = 1; /* index into parent argv vector */
ROKEN_LIB_VARIABLE int optopt;   /* character checked for validity */
int	optreset;                /* reset getopt */
ROKEN_LIB_VARIABLE char	*optarg; /* argument associated with option */

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
getopt(int nargc, char * const *nargv, const char *ostr)
{
	static char *place = EMSG;		/* option letter processing */
	char *oli;			/* option letter list index */
	char *p;

	if (optreset || !*place) {		/* update scanning pointer */
		optreset = 0;
		if (optind >= nargc || *(place = nargv[optind]) != '-') {
			place = EMSG;
			return(-1);
		}
		if (place[1] && *++place == '-') {	/* found "--" */
			++optind;
			place = EMSG;
			return(-1);
		}
	}					/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
	    !(oli = strchr(ostr, optopt))) {
		/*
		 * if the user didn't specify '-' as an option,
		 * assume it means -1 (EOF).
		 */
		if (optopt == (int)'-')
			return(-1);
		if (!*place)
			++optind;
		if (opterr && *ostr != ':') {
			if (!(p = strrchr(*nargv, '/')))
				p = *nargv;
			else
				++p;
			fprintf(stderr, "%s: illegal option -- %c\n",
			    p, optopt);
		}
		return(BADCH);
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
			if (!(p = strrchr(*nargv, '/')))
				p = *nargv;
			else
				++p;
			if (*ostr == ':')
				return(BADARG);
			if (opterr)
				fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    p, optopt);
			return(BADCH);
		}
	 	else				/* white space */
			optarg = nargv[optind];
		place = EMSG;
		++optind;
	}
	return(optopt);				/* dump back option letter */
}