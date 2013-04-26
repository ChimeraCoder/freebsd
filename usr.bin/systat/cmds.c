
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

#ifdef lint
static const char sccsid[] = "@(#)cmds.c	8.2 (Berkeley) 4/29/95";
#endif

#include <sys/param.h>

#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "systat.h"
#include "extern.h"

void
command(const char *cmd)
{
	struct cmdtab *p;
	char *cp, *tmpstr, *tmpstr1;
	double t;

	tmpstr = tmpstr1 = strdup(cmd);
	for (cp = tmpstr1; *cp && !isspace(*cp); cp++)
		;
	if (*cp)
		*cp++ = '\0';
	if (*tmpstr1 == '\0')
		return;
	for (; *cp && isspace(*cp); cp++)
		;
	if (strcmp(tmpstr1, "quit") == 0 || strcmp(tmpstr1, "q") == 0)
		die(0);
	if (strcmp(tmpstr1, "load") == 0) {
		load();
		goto done;
	}
	if (strcmp(tmpstr1, "stop") == 0) {
		delay = 0;
		mvaddstr(CMDLINE, 0, "Refresh disabled.");
		clrtoeol();
		goto done;
	}
	if (strcmp(tmpstr1, "help") == 0) {
		int _col, _len;

		move(CMDLINE, _col = 0);
		for (p = cmdtab; p->c_name; p++) {
			_len = strlen(p->c_name);
			if (_col + _len > COLS)
				break;
			addstr(p->c_name); _col += _len;
			if (_col + 1 < COLS)
				addch(' ');
		}
		clrtoeol();
		goto done;
	}
	t = strtod(tmpstr1, NULL) * 1000000.0;
	if (t > 0 && t < (double)UINT_MAX)
		delay = (unsigned int)t;
	if ((t <= 0 || t > (double)UINT_MAX) &&
	    (strcmp(tmpstr1, "start") == 0 ||
	    strcmp(tmpstr1, "interval") == 0)) {
		if (*cp != '\0') {
			t = strtod(cp, NULL) * 1000000.0;
			if (t <= 0 || t >= (double)UINT_MAX) {
				error("%d: bad interval.", (int)t);
				goto done;
			}
		}
	}
	if (t > 0) {
		delay = (unsigned int)t;
		display();
		status();
		goto done;
	}
	p = lookup(tmpstr1);
	if (p == (struct cmdtab *)-1) {
		error("%s: Ambiguous command.", tmpstr1);
		goto done;
	}
	if (p) {
		if (curcmd == p)
			goto done;
		(*curcmd->c_close)(wnd);
		curcmd->c_flags &= ~CF_INIT;
		wnd = (*p->c_open)();
		if (wnd == 0) {
			error("Couldn't open new display");
			wnd = (*curcmd->c_open)();
			if (wnd == 0) {
				error("Couldn't change back to previous cmd");
				exit(1);
			}
			p = curcmd;
		}
		if ((p->c_flags & CF_INIT) == 0) {
			if ((*p->c_init)())
				p->c_flags |= CF_INIT;
			else
				goto done;
		}
		curcmd = p;
		labels();
		display();
		status();
		goto done;
	}
	if (curcmd->c_cmd == 0 || !(*curcmd->c_cmd)(tmpstr1, cp))
		error("%s: Unknown command.", tmpstr1);
done:
	free(tmpstr);
}

struct cmdtab *
lookup(const char *name)
{
	const char *p, *q;
	struct cmdtab *ct, *found;
	int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = (struct cmdtab *) 0;
	for (ct = cmdtab; (p = ct->c_name); ct++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (ct);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = ct;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmdtab *)-1);
	return (found);
}

void
status(void)
{

	error("Showing %s, refresh every %d seconds.",
	  curcmd->c_name, delay / 1000000);
}

int
prefix(const char *s1, const char *s2)
{

	while (*s1 == *s2) {
		if (*s1 == '\0')
			return (1);
		s1++, s2++;
	}
	return (*s1 == '\0');
}