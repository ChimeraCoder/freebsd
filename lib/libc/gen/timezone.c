
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
static char sccsid[] = "@(#)timezone.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TZ_MAX_CHARS 255

char *_tztab(int, int);

/*
 * timezone --
 *	The arguments are the number of minutes of time you are westward
 *	from Greenwich and whether DST is in effect.  It returns a string
 *	giving the name of the local timezone.  Should be replaced, in the
 *	application code, by a call to localtime.
 */

static char	czone[TZ_MAX_CHARS];		/* space for zone name */

char *
timezone(int zone, int dst)
{
	char	*beg,
			*end;

	if ( (beg = getenv("TZNAME")) ) {	/* set in environment */
		if ((end = strchr(beg, ','))) {	/* "PST,PDT" */
			if (dst)
				return(++end);
			*end = '\0';
			(void)strncpy(czone,beg,sizeof(czone) - 1);
			czone[sizeof(czone) - 1] = '\0';
			*end = ',';
			return(czone);
		}
		return(beg);
	}
	return(_tztab(zone,dst));	/* default: table or created zone */
}

static struct zone {
	int	offset;
	char	*stdzone;
	char	*dlzone;
} zonetab[] = {
	{-1*60,	"MET",	"MET DST"},	/* Middle European */
	{-2*60,	"EET",	"EET DST"},	/* Eastern European */
	{4*60,	"AST",	"ADT"},		/* Atlantic */
	{5*60,	"EST",	"EDT"},		/* Eastern */
	{6*60,	"CST",	"CDT"},		/* Central */
	{7*60,	"MST",	"MDT"},		/* Mountain */
	{8*60,	"PST",	"PDT"},		/* Pacific */
#ifdef notdef
	/* there's no way to distinguish this from WET */
	{0,	"GMT",	0},		/* Greenwich */
#endif
	{0*60,	"WET",	"WET DST"},	/* Western European */
	{-10*60,"EST",	"EST"},		/* Aust: Eastern */
     {-10*60+30,"CST",	"CST"},		/* Aust: Central */
	{-8*60,	"WST",	0},		/* Aust: Western */
	{-1}
};

/*
 * _tztab --
 *	check static tables or create a new zone name; broken out so that
 *	we can make a guess as to what the zone is if the standard tables
 *	aren't in place in /etc.  DO NOT USE THIS ROUTINE OUTSIDE OF THE
 *	STANDARD LIBRARY.
 */
char *
_tztab(int zone, int dst)
{
	struct zone	*zp;
	char	sign;

	for (zp = zonetab; zp->offset != -1;++zp)	/* static tables */
		if (zp->offset == zone) {
			if (dst && zp->dlzone)
				return(zp->dlzone);
			if (!dst && zp->stdzone)
				return(zp->stdzone);
		}

	if (zone < 0) {					/* create one */
		zone = -zone;
		sign = '+';
	}
	else
		sign = '-';
	(void)snprintf(czone, sizeof(czone),
	    "GMT%c%d:%02d",sign,zone / 60,zone % 60);
	return(czone);
}