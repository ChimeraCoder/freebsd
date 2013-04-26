
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
static char sccsid[] = "@(#)getttyent.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <ttyent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static char zapchar;
static FILE *tf;
static size_t lbsize;
static char *line;

#define	MALLOCCHUNK	100

static char *skip(char *);
static char *value(char *);

struct ttyent *
getttynam(const char *tty)
{
	struct ttyent *t;

	if (strncmp(tty, "/dev/", 5) == 0)
		tty += 5;
	setttyent();
	while ( (t = getttyent()) )
		if (!strcmp(tty, t->ty_name))
			break;
	endttyent();
	return (t);
}

struct ttyent *
getttyent(void)
{
	static struct ttyent tty;
	char *p;
	int c;
	size_t i;

	if (!tf && !setttyent())
		return (NULL);
	for (;;) {
		if (!fgets(p = line, lbsize, tf))
			return (NULL);
		/* extend buffer if line was too big, and retry */
		while (!strchr(p, '\n') && !feof(tf)) {
			i = strlen(p);
			lbsize += MALLOCCHUNK;
			if ((p = realloc(line, lbsize)) == NULL) {
				(void)endttyent();
				return (NULL);
			}
			line = p;
			if (!fgets(&line[i], lbsize - i, tf))
				return (NULL);
		}
		while (isspace((unsigned char)*p))
			++p;
		if (*p && *p != '#')
			break;
	}

#define scmp(e) !strncmp(p, e, sizeof(e) - 1) && isspace((unsigned char)p[sizeof(e) - 1])
#define	vcmp(e)	!strncmp(p, e, sizeof(e) - 1) && p[sizeof(e) - 1] == '='

	zapchar = 0;
	tty.ty_name = p;
	tty.ty_status = 0;
	tty.ty_window = NULL;
	tty.ty_group  = _TTYS_NOGROUP;

	p = skip(p);
	if (!*(tty.ty_getty = p))
		tty.ty_getty = tty.ty_type = NULL;
	else {
		p = skip(p);
		if (!*(tty.ty_type = p))
			tty.ty_type = NULL;
		else {
			/* compatibility kludge: handle network/dialup specially */
			if (scmp(_TTYS_DIALUP))
				tty.ty_status |= TTY_DIALUP;
			else if (scmp(_TTYS_NETWORK))
				tty.ty_status |= TTY_NETWORK;
			p = skip(p);
		}
	}

	for (; *p; p = skip(p)) {
		if (scmp(_TTYS_OFF))
			tty.ty_status &= ~TTY_ON;
		else if (scmp(_TTYS_ON))
			tty.ty_status |= TTY_ON;
		else if (scmp(_TTYS_SECURE))
			tty.ty_status |= TTY_SECURE;
		else if (scmp(_TTYS_INSECURE))
			tty.ty_status &= ~TTY_SECURE;
		else if (scmp(_TTYS_DIALUP))
			tty.ty_status |= TTY_DIALUP;
		else if (scmp(_TTYS_NETWORK))
			tty.ty_status |= TTY_NETWORK;
		else if (vcmp(_TTYS_WINDOW))
			tty.ty_window = value(p);
		else if (vcmp(_TTYS_GROUP))
			tty.ty_group = value(p);
		else
			break;
	}

	if (zapchar == '#' || *p == '#')
		while ((c = *++p) == ' ' || c == '\t')
			;
	tty.ty_comment = p;
	if (*p == 0)
		tty.ty_comment = 0;
	if ((p = strchr(p, '\n')))
		*p = '\0';
	return (&tty);
}

#define	QUOTED	1

/*
 * Skip over the current field, removing quotes, and return a pointer to
 * the next field.
 */
static char *
skip(char *p)
{
	char *t;
	int c, q;

	for (q = 0, t = p; (c = *p) != '\0'; p++) {
		if (c == '"') {
			q ^= QUOTED;	/* obscure, but nice */
			continue;
		}
		if (q == QUOTED && *p == '\\' && *(p+1) == '"')
			p++;
		*t++ = *p;
		if (q == QUOTED)
			continue;
		if (c == '#') {
			zapchar = c;
			*p = 0;
			break;
		}
		if (c == '\t' || c == ' ' || c == '\n') {
			zapchar = c;
			*p++ = 0;
			while ((c = *p) == '\t' || c == ' ' || c == '\n')
				p++;
			break;
		}
	}
	*--t = '\0';
	return (p);
}

static char *
value(char *p)
{

	return ((p = strchr(p, '=')) ? ++p : NULL);
}

int
setttyent(void)
{

	if (line == NULL) {
		if ((line = malloc(MALLOCCHUNK)) == NULL)
			return (0);
		lbsize = MALLOCCHUNK;
	}
	if (tf) {
		rewind(tf);
		return (1);
	} else if ( (tf = fopen(_PATH_TTYS, "re")) )
		return (1);
	return (0);
}

int
endttyent(void)
{
	int rval;

	/*
         * NB: Don't free `line' because getttynam()
	 * may still be referencing it
	 */
	if (tf) {
		rval = (fclose(tf) != EOF);
		tf = NULL;
		return (rval);
	}
	return (1);
}

static int
isttystat(const char *tty, int flag)
{
	struct ttyent *t;

	return ((t = getttynam(tty)) == NULL) ? 0 : !!(t->ty_status & flag);
}


int
isdialuptty(const char *tty)
{

	return isttystat(tty, TTY_DIALUP);
}

int
isnettty(const char *tty)
{

	return isttystat(tty, TTY_NETWORK);
}