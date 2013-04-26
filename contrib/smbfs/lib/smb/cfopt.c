
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
#include <sys/param.h>

#include <stdio.h>
#include <string.h>

#include <cflib.h>

extern char *__progname;

int	cf_opterr = 1,		/* if error message should be printed */
	cf_optind = 1,		/* index into parent argv vector */
	cf_optopt,			/* character checked for validity */
	cf_optreset;		/* reset getopt */
const char* cf_optarg;		/* argument associated with option */

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

int
cf_getopt(nargc, nargv, ostr)
	int nargc;
	char * const *nargv;
	const char *ostr;
{
	static const char *place = EMSG;		/* option letter processing */
	char *oli;				/* option letter list index */
	int tmpind;

	if (cf_optreset || !*place) {		/* update scanning pointer */
		cf_optreset = 0;
		tmpind = cf_optind;
		while (1) {
			if (tmpind >= nargc) {
				place = EMSG;
				return (-1);
			}
			if (*(place = nargv[tmpind]) != '-') {
				tmpind++;
				continue;	/* lookup next option */
			}
			if (place[1] && *++place == '-') {	/* found "--" */
				cf_optind = ++tmpind;
				place = EMSG;
				return (-1);
			}
			cf_optind = tmpind;
			break;
		}
	}					/* option letter okay? */
	if ((cf_optopt = (int)*place++) == (int)':' ||
	    !(oli = strchr(ostr, cf_optopt))) {
		/*
		 * if the user didn't specify '-' as an option,
		 * assume it means -1.
		 */
		if (cf_optopt == (int)'-')
			return (-1);
		if (!*place)
			++cf_optind;
		if (cf_opterr && *ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", __progname, cf_optopt);
		return (BADCH);
	}
	if (*++oli != ':') {			/* don't need argument */
		cf_optarg = NULL;
		if (!*place)
			++cf_optind;
	}
	else {					/* need an argument */
		if (*place)			/* no white space */
			cf_optarg = place;
		else if (nargc <= ++cf_optind) {	/* no arg */
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (cf_opterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    __progname, cf_optopt);
			return (BADCH);
		}
	 	else				/* white space */
			cf_optarg = nargv[cf_optind];
		place = EMSG;
		++cf_optind;
	}
	return (cf_optopt);			/* dump back option letter */
}