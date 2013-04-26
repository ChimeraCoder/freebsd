
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

#if !defined(lint) && !defined(SCCSID)
static char sccsid[] = "@(#)parse.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint && not SCCSID */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * parse.c: parse an editline extended command
 *
 * commands are:
 *
 *	bind
 *	echotc
 *	edit
 *	gettc
 *	history
 *	settc
 *	setty
 */
#include "sys.h"
#include "el.h"
#include <stdlib.h>

private const struct {
	const char *name;
	int (*func)(EditLine *, int, const char **);
} cmds[] = {
	{ "bind",	map_bind	},
	{ "echotc",	term_echotc	},
	{ "edit",	el_editmode	},
	{ "history",	hist_command	},
	{ "telltc",	term_telltc	},
	{ "settc",	term_settc	},
	{ "setty",	tty_stty	},
	{ NULL,		NULL		}
};


/* parse_line():
 *	Parse a line and dispatch it
 */
protected int
parse_line(EditLine *el, const char *line)
{
	const char **argv;
	int argc;
	Tokenizer *tok;

	tok = tok_init(NULL);
	tok_str(tok, line, &argc, &argv);
	argc = el_parse(el, argc, argv);
	tok_end(tok);
	return (argc);
}


/* el_parse():
 *	Command dispatcher
 */
public int
el_parse(EditLine *el, int argc, const char *argv[])
{
	const char *ptr;
	int i;

	if (argc < 1)
		return (-1);
	ptr = strchr(argv[0], ':');
	if (ptr != NULL) {
		char *tprog;
		size_t l;

		if (ptr == argv[0])
			return (0);
		l = ptr - argv[0] - 1;
		tprog = (char *) el_malloc(l + 1);
		if (tprog == NULL)
			return (0);
		(void) strncpy(tprog, argv[0], l);
		tprog[l] = '\0';
		ptr++;
		l = el_match(el->el_prog, tprog);
		el_free(tprog);
		if (!l)
			return (0);
	} else
		ptr = argv[0];

	for (i = 0; cmds[i].name != NULL; i++)
		if (strcmp(cmds[i].name, ptr) == 0) {
			i = (*cmds[i].func) (el, argc, argv);
			return (-i);
		}
	return (-1);
}


/* parse__escape():
 *	Parse a string of the form ^<char> \<odigit> \<char> and return
 *	the appropriate character or -1 if the escape is not valid
 */
protected int
parse__escape(const char **ptr)
{
	const char *p;
	int c;

	p = *ptr;

	if (p[1] == 0)
		return (-1);

	if (*p == '\\') {
		p++;
		switch (*p) {
		case 'a':
			c = '\007';	/* Bell */
			break;
		case 'b':
			c = '\010';	/* Backspace */
			break;
		case 't':
			c = '\011';	/* Horizontal Tab */
			break;
		case 'n':
			c = '\012';	/* New Line */
			break;
		case 'v':
			c = '\013';	/* Vertical Tab */
			break;
		case 'f':
			c = '\014';	/* Form Feed */
			break;
		case 'r':
			c = '\015';	/* Carriage Return */
			break;
		case 'e':
			c = '\033';	/* Escape */
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		{
			int cnt, ch;

			for (cnt = 0, c = 0; cnt < 3; cnt++) {
				ch = *p++;
				if (ch < '0' || ch > '7') {
					p--;
					break;
				}
				c = (c << 3) | (ch - '0');
			}
			if ((c & 0xffffff00) != 0)
				return (-1);
			--p;
			break;
		}
		default:
			c = *p;
			break;
		}
	} else if (*p == '^') {
		p++;
		c = (*p == '?') ? '\177' : (*p & 0237);
	} else
		c = *p;
	*ptr = ++p;
	return ((unsigned char)c);
}

/* parse__string():
 *	Parse the escapes from in and put the raw string out
 */
protected char *
parse__string(char *out, const char *in)
{
	char *rv = out;
	int n;

	for (;;)
		switch (*in) {
		case '\0':
			*out = '\0';
			return (rv);

		case '\\':
		case '^':
			if ((n = parse__escape(&in)) == -1)
				return (NULL);
			*out++ = n;
			break;

		case 'M':
			if (in[1] == '-' && in[2] != '\0') {
				*out++ = '\033';
				in += 2;
				break;
			}
			/*FALLTHROUGH*/

		default:
			*out++ = *in++;
			break;
		}
}


/* parse_cmd():
 *	Return the command number for the command string given
 *	or -1 if one is not found
 */
protected int
parse_cmd(EditLine *el, const char *cmd)
{
	el_bindings_t *b;

	for (b = el->el_map.help; b->name != NULL; b++)
		if (strcmp(b->name, cmd) == 0)
			return (b->func);
	return (-1);
}