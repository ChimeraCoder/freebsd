
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
#if 0
static char sccsid[] = "@(#)cchar.c	8.5 (Berkeley) 4/2/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <err.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "stty.h"
#include "extern.h"

static int c_cchar(const void *, const void *);

/*
 * Special control characters.
 *
 * Cchars1 are the standard names, cchars2 are the old aliases.
 * The first are displayed, but both are recognized on the
 * command line.
 */
struct cchar cchars1[] = {
	{ "discard",	VDISCARD, 	CDISCARD },
	{ "dsusp", 	VDSUSP,		CDSUSP },
	{ "eof",	VEOF,		CEOF },
	{ "eol",	VEOL,		CEOL },
	{ "eol2",	VEOL2,		CEOL },
	{ "erase",	VERASE,		CERASE },
	{ "erase2",	VERASE2,	CERASE2 },
	{ "intr",	VINTR,		CINTR },
	{ "kill",	VKILL,		CKILL },
	{ "lnext",	VLNEXT,		CLNEXT },
	{ "min",	VMIN,		CMIN },
	{ "quit",	VQUIT,		CQUIT },
	{ "reprint",	VREPRINT, 	CREPRINT },
	{ "start",	VSTART,		CSTART },
	{ "status",	VSTATUS, 	CSTATUS },
	{ "stop",	VSTOP,		CSTOP },
	{ "susp",	VSUSP,		CSUSP },
	{ "time",	VTIME,		CTIME },
	{ "werase",	VWERASE,	CWERASE },
	{ NULL,		0,		0},
};

struct cchar cchars2[] = {
	{ "brk",	VEOL,		CEOL },
	{ "flush",	VDISCARD, 	CDISCARD },
	{ "rprnt",	VREPRINT, 	CREPRINT },
	{ NULL,		0,		0 },
};

static int
c_cchar(const void *a, const void *b)
{

        return (strcmp(((const struct cchar *)a)->name, ((const struct cchar *)b)->name));
}

int
csearch(char ***argvp, struct info *ip)
{
	struct cchar *cp, tmp;
	long val;
	char *arg, *ep, *name;

	name = **argvp;

	tmp.name = name;
	if (!(cp = (struct cchar *)bsearch(&tmp, cchars1,
	    sizeof(cchars1)/sizeof(struct cchar) - 1, sizeof(struct cchar),
	    c_cchar)) && !(cp = (struct cchar *)bsearch(&tmp, cchars2,
	    sizeof(cchars2)/sizeof(struct cchar) - 1, sizeof(struct cchar),
	    c_cchar)))
		return (0);

	arg = *++*argvp;
	if (!arg) {
		warnx("option requires an argument -- %s", name);
		usage();
	}

#define CHK(s)  (*arg == s[0] && !strcmp(arg, s))
	if (CHK("undef") || CHK("<undef>"))
		ip->t.c_cc[cp->sub] = _POSIX_VDISABLE;
	else if (cp->sub == VMIN || cp->sub == VTIME) {
		val = strtol(arg, &ep, 10);
		if (val > UCHAR_MAX) {
			warnx("maximum option value is %d -- %s",
			    UCHAR_MAX, name);
			usage();
		}
		if (*ep != '\0') {
			warnx("option requires a numeric argument -- %s", name);
			usage();
		}
		ip->t.c_cc[cp->sub] = val;
	} else if (arg[0] == '^')
		ip->t.c_cc[cp->sub] = (arg[1] == '?') ? 0177 :
		    (arg[1] == '-') ? _POSIX_VDISABLE : arg[1] & 037;
	else
		ip->t.c_cc[cp->sub] = arg[0];
	ip->set = 1;
	return (1);
}