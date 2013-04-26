
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

/* OPENBSD ORIGINAL: lib/libc/stdlib/getopt.c */

#include "includes.h"
#if !defined(HAVE_GETOPT) || !defined(HAVE_GETOPT_OPTRESET)

#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$OpenBSD: getopt.c,v 1.5 2003/06/02 20:18:37 millert Exp $";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	BSDopterr = 1,		/* if error message should be printed */
	BSDoptind = 1,		/* index into parent argv vector */
	BSDoptopt,		/* character checked for validity */
	BSDoptreset;		/* reset getopt */
char	*BSDoptarg;		/* argument associated with option */

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

/*
 * getopt --
 *	Parse argc/argv argument vector.
 */
int
BSDgetopt(nargc, nargv, ostr)
	int nargc;
	char * const *nargv;
	const char *ostr;
{
	extern char *__progname;
	static char *place = EMSG;		/* option letter processing */
	char *oli;				/* option letter list index */

	if (ostr == NULL)
		return (-1);

	if (BSDoptreset || !*place) {		/* update scanning pointer */
		BSDoptreset = 0;
		if (BSDoptind >= nargc || *(place = nargv[BSDoptind]) != '-') {
			place = EMSG;
			return (-1);
		}
		if (place[1] && *++place == '-') {	/* found "--" */
			++BSDoptind;
			place = EMSG;
			return (-1);
		}
	}					/* option letter okay? */
	if ((BSDoptopt = (int)*place++) == (int)':' ||
	    !(oli = strchr(ostr, BSDoptopt))) {
		/*
		 * if the user didn't specify '-' as an option,
		 * assume it means -1.
		 */
		if (BSDoptopt == (int)'-')
			return (-1);
		if (!*place)
			++BSDoptind;
		if (BSDopterr && *ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", __progname, BSDoptopt);
		return (BADCH);
	}
	if (*++oli != ':') {			/* don't need argument */
		BSDoptarg = NULL;
		if (!*place)
			++BSDoptind;
	}
	else {					/* need an argument */
		if (*place)			/* no white space */
			BSDoptarg = place;
		else if (nargc <= ++BSDoptind) {	/* no arg */
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (BSDopterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    __progname, BSDoptopt);
			return (BADCH);
		}
	 	else				/* white space */
			BSDoptarg = nargv[BSDoptind];
		place = EMSG;
		++BSDoptind;
	}
	return (BSDoptopt);			/* dump back option letter */
}

#endif /* !defined(HAVE_GETOPT) || !defined(HAVE_OPTRESET) */