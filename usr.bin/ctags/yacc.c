
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)yacc.c	8.3 (Berkeley) 4/2/94";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <ctype.h>
#include <limits.h>
#include <stdio.h>

#include "ctags.h"

/*
 * y_entries:
 *	find the yacc tags and put them in.
 */
void
y_entries(void)
{
	int	c;
	char	*sp;
	bool	in_rule;
	char	tok[MAXTOKEN];

	in_rule = NO;

	while (GETC(!=, EOF))
		switch (c) {
		case '\n':
			SETLINE;
			/* FALLTHROUGH */
		case ' ':
		case '\f':
		case '\r':
		case '\t':
			break;
		case '{':
			if (skip_key('}'))
				in_rule = NO;
			break;
		case '\'':
		case '"':
			if (skip_key(c))
				in_rule = NO;
			break;
		case '%':
			if (GETC(==, '%'))
				return;
			(void)ungetc(c, inf);
			break;
		case '/':
			if (GETC(==, '*') || c == '/')
				skip_comment(c);
			else
				(void)ungetc(c, inf);
			break;
		case '|':
		case ';':
			in_rule = NO;
			break;
		default:
			if (in_rule || (!isalpha(c) && c != '.' && c != '_'))
				break;
			sp = tok;
			*sp++ = c;
			while (GETC(!=, EOF) && (intoken(c) || c == '.'))
				*sp++ = c;
			*sp = EOS;
			getline();		/* may change before ':' */
			while (iswhite(c)) {
				if (c == '\n')
					SETLINE;
				if (GETC(==, EOF))
					return;
			}
			if (c == ':') {
				pfnote(tok, lineno);
				in_rule = YES;
			}
			else
				(void)ungetc(c, inf);
		}
}

/*
 * toss_yysec --
 *	throw away lines up to the next "\n%%\n"
 */
void
toss_yysec(void)
{
	int	c;			/* read character */
	int	state;

	/*
	 * state == 0 : waiting
	 * state == 1 : received a newline
	 * state == 2 : received first %
	 * state == 3 : received second %
	 */
	lineftell = ftell(inf);
	for (state = 0; GETC(!=, EOF);)
		switch (c) {
		case '\n':
			++lineno;
			lineftell = ftell(inf);
			if (state == 3)		/* done! */
				return;
			state = 1;		/* start over */
			break;
		case '%':
			if (state)		/* if 1 or 2 */
				++state;	/* goto 3 */
			break;
		default:
			state = 0;		/* reset */
			break;
		}
}